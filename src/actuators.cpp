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
    {PUMP_SIRAM_PIN, "Penyiraman", COMMAND_TOPIC_PUMP_SIRAM, STATE_TOPIC_PUMP_SIRAM, false, 0}};

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

// --- Forward Declarations for Static (Private) Functions ---
static void control_pump_by_volume(Pump& pump, float volume_ml);
static void control_pump_by_duration(Pump& pump, unsigned long duration_ms);
static bool are_any_pumps_running();

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

void actuators_loop() {
  unsigned long currentTime = millis();
  for (int i = 0; i < NUM_PUMPS; i++) {
    // Check if a timed run for this pump needs to be stopped
    if (pumps[i].isOn && pumps[i].stopTime > 0 && currentTime >= pumps[i].stopTime) {
      digitalWrite(pumps[i].pin, LOW);
      pumps[i].isOn = false;
      pumps[i].stopTime = 0;
      LOG_PRINTF("[Pump] %s finished timed run.\n", pumps[i].name);
      mqtt_publish_state(pumps[i].stateTopic, PAYLOAD_OFF, true);
    }
  }
}

void actuators_handle_pump_command(const char* topic, const char* command) {
  for (int i = 0; i < NUM_PUMPS; i++) {
    // Find the pump that corresponds to the received topic
    if (pumps[i].commandTopic == topic) {
      // Use case-insensitive compare for the "OFF" command for robustness
      if (strcasecmp(command, "OFF") == 0) {
        digitalWrite(pumps[i].pin, LOW);
        pumps[i].isOn = false;
        pumps[i].stopTime = 0; // Cancel any timed run
        LOG_PRINTF("[Pump] %s force stopped via MQTT.\n", pumps[i].name);
        mqtt_publish_state(pumps[i].stateTopic, PAYLOAD_OFF, true);
      } else if (pumps[i].pin == PUMP_SIRAM_PIN) {
        // For the watering pump, the command is a duration in seconds
        float duration_s = atof(command);
        if (duration_s > 0) {
          control_pump_by_duration(pumps[i], duration_s * 1000);
        }
      } else {
        // For all other pumps, the command is a volume in ml
        float volume_ml = atof(command);
        if (volume_ml > 0) {
          control_pump_by_volume(pumps[i], volume_ml);
        }
      }
      return; // Command was handled, no need to check other pumps
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
  // An alert should be active if the water level reading is invalid (NAN) or below the critical threshold.
  bool shouldAlertBeActive = isnan(values.waterLevelCm) || (values.waterLevelCm <= WATER_LEVEL_CRITICAL_CM);

  // State change: from normal to alert
  if (shouldAlertBeActive && !isWaterLevelAlertActive) {
    isWaterLevelAlertActive = true;
    char alertMessage[80];
    if (isnan(values.waterLevelCm)) {
      strcpy(alertMessage, "ALERT: Water level sensor reading is invalid!");
    } else {
      sprintf(alertMessage, "ALERT: Water level is critical! Current: %.1f cm", values.waterLevelCm);
    }
    LOG_PRINTF(">>> ALERT: %s <<<\n", alertMessage);
    mqtt_publish_alert(alertMessage);
  } 
  // State change: from alert to normal
  else if (!shouldAlertBeActive && isWaterLevelAlertActive) {
    isWaterLevelAlertActive = false;
    LOG_PRINTLN(">>> INFO: Water level has returned to normal. <<<");
    mqtt_publish_alert("OK: Water level is normal.");
  }

  // Set the buzzer state based on the alert status.
  digitalWrite(BUZZER_PIN, isWaterLevelAlertActive ? HIGH : LOW);
}

void actuators_publish_states() {
  LOG_PRINTLN("[Actuators] Publishing initial states to MQTT...");
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
    LOG_PRINTF("[Pump] Cannot start %s, another pump is running.\n", pump.name);
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
    LOG_PRINTF("[Pump] Cannot start %s, another pump is running.\n", pump.name);
    return;
  }

  LOG_PRINTF("[Pump] Running %s for %lu ms.\n", pump.name, duration_ms);
  pump.stopTime = millis() + duration_ms;
  digitalWrite(pump.pin, HIGH);
  pump.isOn = true;
  mqtt_publish_state(pump.stateTopic, PAYLOAD_ON, true);
}
