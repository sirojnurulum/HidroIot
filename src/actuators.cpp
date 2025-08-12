/**
 * @file actuators.cpp
 * @brief Implements the logic for controlling all physical actuators.
 *
 * This module manages the state of pumps and the buzzer, handles commands
 * to run pumps for specific volumes or durations, and manages system-wide
 * alerts based on sensor data.
 */

#include "actuators.h"
#include "config.h"
#include "mqtt_handler.h" // For publishing alerts and state
#include <stdlib.h>       // For atof()
#include <strings.h>      // For strcasecmp()
#include <cstring>        // For strcmp()

// --- Module-Private (Static) Constants & Variables ---

/// @brief MQTT payload for the "ON" state.
static const char* PAYLOAD_ON = "ON";
/// @brief MQTT payload for the "OFF" state.
static const char* PAYLOAD_OFF = "OFF";

/**
 * @struct Pump
 * @brief Holds all state and configuration for a single pump.
 */
struct Pump {
  const int pin;              ///< The GPIO pin connected to the pump's relay.
  const char* name;           ///< A human-readable name for logging.
  const std::string commandTopic; ///< The MQTT topic to receive commands on.
  const std::string stateTopic;   ///< The MQTT topic to publish state to.
  bool isOn;                  ///< The current state of the pump (true if running).
  unsigned long stopTime;     ///< The time (from millis()) when a timed run should stop. 0 if not in a timed run.
};

/// @brief A unified array of all pumps for easy, scalable management.
static Pump pumps[] = {
    {PUMP_NUTRISI_A_PIN, "Nutrisi A", COMMAND_TOPIC_PUMP_A, STATE_TOPIC_PUMP_A, false, 0},
    {PUMP_NUTRISI_B_PIN, "Nutrisi B", COMMAND_TOPIC_PUMP_B, STATE_TOPIC_PUMP_B, false, 0},
    {PUMP_PH_PIN, "pH", COMMAND_TOPIC_PUMP_PH, STATE_TOPIC_PUMP_PH, false, 0},
    {PUMP_SIRAM_PIN, "Penyiraman", COMMAND_TOPIC_PUMP_SIRAM, STATE_TOPIC_PUMP_SIRAM, false, 0},
    {PUMP_TANDON_PIN, "Pengisian Tandon", COMMAND_TOPIC_PUMP_TANDON, STATE_TOPIC_PUMP_TANDON, false, 0}};

/// @brief The total number of pumps, calculated automatically from the array size.
static const int NUM_PUMPS = sizeof(pumps) / sizeof(pumps[0]);

/// @brief Tracks if the water level alert is currently active to prevent spamming alerts.
static bool isWaterLevelAlertActive = false;

/// @brief Defines the operational modes of the system.
enum SystemMode {
    NUTRITION, ///< Normal mode, all functions enabled.
    CLEANER    ///< Maintenance mode, potentially disabling auto-dosing (future feature).
};
/// @brief The current operational mode of the system.
static SystemMode currentSystemMode = NUTRITION;

// --- Global Automation State ---
/// @brief Global instance of automation state, initialized to default values
AutomationState automation_state = {
    .auto_dosing_enabled = false,
    .auto_refill_enabled = false,  
    .auto_irrigation_enabled = false
};

// --- Forward Declarations for Static (Private) Functions ---
static void control_pump_by_volume(Pump& pump, float volume_ml);
static void control_pump_by_duration(Pump& pump, unsigned long duration_ms);
static bool are_any_pumps_running();
static void check_tandon_safety(const SensorValues& currentValues);

// --- Public Function Implementations ---

void actuators_init() {
  LOG_PRINTLN("[Actuators] Initializing...");
  for (int i = 0; i < NUM_PUMPS; i++) {
    pinMode(pumps[i].pin, OUTPUT);
    digitalWrite(pumps[i].pin, LOW); // Ensure all pumps are OFF
  }
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); // Ensure buzzer is OFF
}

void actuators_loop(const SensorValues& currentValues) {
  unsigned long currentTime = millis();
  for (int i = 0; i < NUM_PUMPS; i++) {
    // Check if a timed run for this pump needs to be stopped
    if (pumps[i].isOn && pumps[i].stopTime > 0 && currentTime >= pumps[i].stopTime) {
      digitalWrite(pumps[i].pin, LOW);
      pumps[i].isOn = false;
      pumps[i].stopTime = 0;
      LOG_PRINTF("[Actuator] %s finished timed run.\n", pumps[i].name);
      mqtt_publish_state(pumps[i].stateTopic, PAYLOAD_OFF, true);
    }
  }

  // Continuously check for tandon overflow safety, regardless of automation state.
  check_tandon_safety(currentValues);
}

void actuators_handle_pump_command(const char* topic, const char* command) {
  // Find the pump that corresponds to the received topic
  for (int i = 0; i < NUM_PUMPS; i++) {
    // Use strcmp for a robust comparison between the std::string from our config
    // and the char* from the MQTT library. This is safer than direct `==` comparison.
    if (strcmp(pumps[i].commandTopic.c_str(), topic) == 0) {
      LOG_PRINTF("[Actuator] Command '%s' received for pump '%s'\n", command, pumps[i].name);

      // First, handle the universal "OFF" command for any pump.
      if (strcasecmp(command, "OFF") == 0) {
        digitalWrite(pumps[i].pin, LOW);
        pumps[i].isOn = false;
        pumps[i].stopTime = 0; // Cancel any timed run
        LOG_PRINTF("  > Action: Turning OFF %s.\n", pumps[i].name);
        mqtt_publish_state(pumps[i].stateTopic, PAYLOAD_OFF, true);
        return; // Command handled
      }

      // If not "OFF", handle the specific command based on the pump type using its pin.
      // This is a much more robust and clear approach than a long if-else-if chain.
      switch (pumps[i].pin) {
        case PUMP_TANDON_PIN:
          // The Tandon pump only accepts "ON".
          if (strcasecmp(command, "ON") == 0) {
            LOG_PRINTF("  > Action: Turning ON %s.\n", pumps[i].name);
            digitalWrite(pumps[i].pin, HIGH);
            pumps[i].isOn = true;
            mqtt_publish_state(pumps[i].stateTopic, PAYLOAD_ON, true);
          } else {
            LOG_PRINTF("  > WARN: Invalid command for Tandon pump. Expected 'ON' or 'OFF'. Got '%s'.\n", command);
          }
          break;

        case PUMP_SIRAM_PIN:
          // The watering pump expects a duration in seconds.
          { // Use a block to create a local variable
            float duration_s = atof(command);
            if (duration_s > 0) {
              LOG_PRINTF("  > Action: Running %s for %.1f seconds.\n", pumps[i].name, duration_s);
              control_pump_by_duration(pumps[i], duration_s * 1000);
            } else {
              LOG_PRINTF("  > WARN: Invalid duration for Siram pump. Expected a positive number. Got '%s'.\n", command);
            }
          }
          break;

        default: // Handles PUMP_NUTRISI_A, PUMP_NUTRISI_B, PUMP_PH
          // Dosing pumps expect a volume in milliliters.
          { // Use a block to create a local variable
            float volume_ml = atof(command);
            if (volume_ml > 0) {
              LOG_PRINTF("  > Action: Dosing %.1f ml with %s.\n", volume_ml, pumps[i].name);
              control_pump_by_volume(pumps[i], volume_ml);
            } else {
              LOG_PRINTF("  > WARN: Invalid volume for Dosing pump. Expected a positive number. Got '%s'.\n", command);
            }
          }
          break;
      }
      return; // Command handled
    }
  }
  LOG_PRINTF("[Actuators] WARN: Received command on unhandled topic: %s\n", topic);
}

void actuators_handle_mode_command(const char* command) {
  bool modeChanged = false;
  const char* newModeStr = nullptr;

  if (strcasecmp(command, "NUTRITION") == 0) {
    if (currentSystemMode != NUTRITION) {
      currentSystemMode = NUTRITION;
      modeChanged = true;
      newModeStr = "NUTRITION";
    }
  } else if (strcasecmp(command, "CLEANER") == 0) {
    if (currentSystemMode != CLEANER) {
      currentSystemMode = CLEANER;
      modeChanged = true;
      newModeStr = "CLEANER";
    }
  } else {
    LOG_PRINTF("[Mode] WARN: Received unknown mode command: %s\n", command);
    return;
  }

  if (modeChanged) {
    LOG_PRINTF("[Mode] System mode changed to %s\n", newModeStr);
    mqtt_publish_state(STATE_TOPIC_SYSTEM_MODE, newModeStr, true);
  } else {
    LOG_PRINTF("[Mode] System already in %s mode.\n", command);
  }
}

void actuators_update_alert_status(const SensorValues& values) {
  // Only raise audible alerts in NUTRITION mode. In CLEANER mode, suppress alerts/buzzer.
  bool alarmsEnabled = (currentSystemMode == NUTRITION);

  // An alert should be active if the water level reading is invalid (NAN) or below the critical threshold.
  bool shouldAlertBeActive = alarmsEnabled && (isnan(values.waterLevelCm) || (values.waterLevelCm <= WATER_LEVEL_CRITICAL_CM));

  // State change: from normal to alert
  if (shouldAlertBeActive && !isWaterLevelAlertActive) {
    isWaterLevelAlertActive = true;
    char alertMessage[100];
    if (isnan(values.waterLevelCm)) {
      strcpy(alertMessage, "ALERT: Water level sensor reading is invalid!");
    } else {
      sprintf(alertMessage, "ALERT: Water level is critical! Current: %.1f cm", values.waterLevelCm);
    }
    LOG_PRINTF(">>> ALERT: %s <<<\n", alertMessage);
    mqtt_publish_alert(alertMessage);
  }
  // State change: from alert to normal
  else if ((!shouldAlertBeActive) && isWaterLevelAlertActive) {
    isWaterLevelAlertActive = false;
    if (alarmsEnabled) {
      LOG_PRINTLN(">>> INFO: Water level has returned to normal. <<<");
      mqtt_publish_alert("OK: Water level is normal.");
    } else {
      LOG_PRINTLN(">>> INFO: Alerts suppressed in CLEANER mode. <<<");
    }
  }

  // Ensure buzzer reflects current alert state; force OFF in CLEANER mode.
  digitalWrite(BUZZER_PIN, (alarmsEnabled && isWaterLevelAlertActive) ? HIGH : LOW);
}

void actuators_publish_states() {
  LOG_PRINTLN("[Actuators] Syncing current actuator states to MQTT...");
  for (int i = 0; i < NUM_PUMPS; i++) {
    mqtt_publish_state(pumps[i].stateTopic, pumps[i].isOn ? PAYLOAD_ON : PAYLOAD_OFF, true);
  }
  mqtt_publish_state(STATE_TOPIC_SYSTEM_MODE, currentSystemMode == NUTRITION ? "NUTRITION" : "CLEANER", true);
}


// --- Static (Private) Function Implementations ---

/**
 * @brief Checks if any pump is currently running.
 * @return true if a pump is active, false otherwise.
 */
static bool are_any_pumps_running() {
  for (int i = 0; i < NUM_PUMPS; i++) {
    if (pumps[i].isOn) return true;
  }
  return false;
}

/**
 * @brief Starts a pump to run for a duration calculated from a volume.
 * @param pump The pump to control.
 * @param volume_ml The volume in milliliters to dispense.
 */
static void control_pump_by_volume(Pump& pump, float volume_ml) {
  if (volume_ml <= 0) return;
  
  // Safety check: Do not start a new pump if one is already running.
  if (are_any_pumps_running()) {
    LOG_PRINTF("[Actuator] Cannot start %s, another pump is running.\n", pump.name);
    return;
  }
  
  unsigned long duration_ms = volume_ml * PUMP_MS_PER_ML;
  control_pump_by_duration(pump, duration_ms);
}

/**
 * @brief Starts a pump to run for a specific duration. This is the core pump control function.
 * @param pump The pump to control.
 * @param duration_ms The duration in milliseconds to run the pump.
 */
static void control_pump_by_duration(Pump& pump, unsigned long duration_ms) {
  if (duration_ms <= 0) return;

  // Safety check: Do not start a new pump if one is already running.
  if (are_any_pumps_running()) {
    LOG_PRINTF("[Actuator] Cannot start %s, another pump is running.\n", pump.name);
    return;
  }

  LOG_PRINTF("[Actuator] Running %s for %lu ms.\n", pump.name, duration_ms);
  pump.stopTime = millis() + duration_ms;
  digitalWrite(pump.pin, HIGH);
  pump.isOn = true;
  mqtt_publish_state(pump.stateTopic, PAYLOAD_ON, true);
}

// --- Automation Functions ---

/**
 * @brief Enforces the reservoir overflow safety rule.
 * This function is called continuously from the main loop to ensure the
 * refill pump/valve is turned off if the water level exceeds the high
 * threshold, regardless of how it was turned on (manually or automatically).
 * @param currentValues The latest sensor readings.
 */
static void check_tandon_safety(const SensorValues& currentValues) {
    // Find the tandon pump
    for (int i = 0; i < NUM_PUMPS; i++) {
        if (pumps[i].pin == PUMP_TANDON_PIN) {
            // If the pump is ON and the water level is valid and has exceeded the high limit
            // PENTING: Nilai 95.0 cm ini adalah jaring pengaman darurat di level firmware.
            // Nilai ini HARUS LEBIH TINGGI dari `input_number.greenhouse_a_refill_target_high`
            // di Home Assistant untuk mencegah tandon meluap jika automasi HA gagal.
            const float FIRMWARE_SAFETY_LEVEL_CM = 95.0;
            if (pumps[i].isOn && !isnan(currentValues.waterLevelCm) && currentValues.waterLevelCm >= FIRMWARE_SAFETY_LEVEL_CM) {
                LOG_PRINTLN("[Actuator] SAFETY OVERRIDE: Tandon level reached high limit. Forcing pump OFF.");
                digitalWrite(pumps[i].pin, LOW);
                pumps[i].isOn = false;
                pumps[i].stopTime = 0;
                mqtt_publish_state(pumps[i].stateTopic, PAYLOAD_OFF, true);
            }
            return; // Found the pump, no need to continue loop
        }
    }
}

void actuators_handle_automation_command(const char* topic, const char* command) {
    LOG_PRINTF("\n[Automation] Command received on topic: %s\n", topic);
    LOG_PRINTF("  > Payload: %s\n", command);

    std::string topic_str(topic);
    bool enable_state = (strcasecmp(command, "ON") == 0);

    if (topic_str == COMMAND_TOPIC_AUTO_DOSING) {
        automation_state.auto_dosing_enabled = enable_state;
        LOG_PRINTF("[Automation] Auto-dosing pH & TDS: %s\n", enable_state ? "ENABLED" : "DISABLED");
        mqtt_publish_state(STATE_TOPIC_AUTO_DOSING, enable_state ? PAYLOAD_ON : PAYLOAD_OFF, true);
        
    } else if (topic_str == COMMAND_TOPIC_AUTO_REFILL) {
        automation_state.auto_refill_enabled = enable_state;
        LOG_PRINTF("[Automation] Auto-refill tandon: %s\n", enable_state ? "ENABLED" : "DISABLED");
        mqtt_publish_state(STATE_TOPIC_AUTO_REFILL, enable_state ? PAYLOAD_ON : PAYLOAD_OFF, true);
        
    } else if (topic_str == COMMAND_TOPIC_AUTO_IRRIGATION) {
        automation_state.auto_irrigation_enabled = enable_state;
        LOG_PRINTF("[Automation] Auto-irrigation: %s\n", enable_state ? "ENABLED" : "DISABLED");
        mqtt_publish_state(STATE_TOPIC_AUTO_IRRIGATION, enable_state ? PAYLOAD_ON : PAYLOAD_OFF, true);
        
    } else {
        LOG_PRINTF("[Automation] Unknown automation topic: %s\n", topic);
    }
}

void actuators_publish_automation_states() {
    LOG_PRINTLN("[Actuators] Publishing automation states to MQTT...");
    
    mqtt_publish_state(STATE_TOPIC_AUTO_DOSING, 
                      automation_state.auto_dosing_enabled ? PAYLOAD_ON : PAYLOAD_OFF, true);
                      
    mqtt_publish_state(STATE_TOPIC_AUTO_REFILL, 
                      automation_state.auto_refill_enabled ? PAYLOAD_ON : PAYLOAD_OFF, true);
                      
    mqtt_publish_state(STATE_TOPIC_AUTO_IRRIGATION, 
                      automation_state.auto_irrigation_enabled ? PAYLOAD_ON : PAYLOAD_OFF, true);
}
