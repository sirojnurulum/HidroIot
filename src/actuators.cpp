#include "actuators.h"
#include "config.h"
#include "mqtt_handler.h" // For publishing alerts and state
#include <stdlib.h>       // For atof()
#include <strings.h>      // For strcasecmp()

// --- Module-Private (Static) Variables ---

/**
 * @struct Pump
 * @brief Holds all state and configuration for a single peristaltic pump.
 */
struct Pump {
  const int pin;
  const char* name;
  const std::string commandTopic;
  const std::string stateTopic;
  bool isOn;
  unsigned long stopTime;
};

// Create an array of all pumps for easy, scalable management
#if HYDROPONIC_INSTANCE == 1 // Produksi
static Pump pumps[] = {
  {PUMP_NUTRISI_A_PIN, "Nutrisi A", COMMAND_TOPIC_PUMP_A, STATE_TOPIC_PUMP_A, false, 0},
  {PUMP_NUTRISI_B_PIN, "Nutrisi B", COMMAND_TOPIC_PUMP_B, STATE_TOPIC_PUMP_B, false, 0},
  {PUMP_PH_PIN, "pH", COMMAND_TOPIC_PUMP_PH, STATE_TOPIC_PUMP_PH, false, 0}
};
#elif HYDROPONIC_INSTANCE == 2 // Penyemaian (Watering Only)
static Pump pumps[] = {
  {PUMP_SIRAM_PIN, "Penyiraman", COMMAND_TOPIC_PUMP_SIRAM, STATE_TOPIC_PUMP_SIRAM, false, 0}
};
#endif
// Calculate the number of pumps automatically
static const int NUM_PUMPS = sizeof(pumps) / sizeof(pumps[0]);

#if HYDROPONIC_INSTANCE == 1
// State variables specific to the 'produksi' instance
static bool isWaterLevelAlertActive = false;
static unsigned long lastAutoDoseCheckTime = 0;

enum AutoDoseState { IDLE, WAITING_FOR_B_DOSE };
static AutoDoseState autoDoseState = IDLE;

enum SystemMode { NUTRITION, CLEANER };
static SystemMode currentSystemMode = NUTRITION;
#endif

// --- Forward Declarations for Static Functions ---
static void control_pump(Pump& pump, float volume_ml);
static void control_pump_for_duration(Pump& pump, unsigned long duration_ms);
static bool are_any_pumps_running();

// --- Public Function Implementations ---

void actuators_init() {
  LOG_PRINTLN("[Actuators] Initializing...");
  for (int i = 0; i < NUM_PUMPS; i++) {
    pinMode(pumps[i].pin, OUTPUT);
    digitalWrite(pumps[i].pin, LOW); // Ensure all pumps are OFF
  }
#if HYDROPONIC_INSTANCE == 1
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); // Ensure buzzer is OFF
#endif
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
      mqtt_publish_state(pumps[i].stateTopic, "OFF", true);

#if HYDROPONIC_INSTANCE == 1
      // If this was part of an auto-dosing sequence, trigger the next part
      if (autoDoseState == WAITING_FOR_B_DOSE && i == 0) { // Pump A just finished
        LOG_PRINTLN("[Auto-Dose] Dosing Nutrient B as part of sequence.");
        if (!are_any_pumps_running()) {
          control_pump(pumps[1], DOSING_AMOUNT_ML); // Dose Nutrient B
        }
        autoDoseState = IDLE; // The sequence is now complete
      }
#endif
    }
  }
}

void actuators_handle_pump_command(const char* topic, const char* command) {
  for (int i = 0; i < NUM_PUMPS; i++) {
    if (strcmp(topic, pumps[i].commandTopic.c_str()) == 0) {
      // Use case-insensitive compare for the "OFF" command
      if (strcasecmp(command, "OFF") == 0) {
        digitalWrite(pumps[i].pin, LOW);
        pumps[i].isOn = false;
        pumps[i].stopTime = 0; // Cancel any timed run
        LOG_PRINTF("[Pump] %s force stopped via MQTT.\n", pumps[i].name);
        mqtt_publish_state(pumps[i].stateTopic, "OFF", true);
#if HYDROPONIC_INSTANCE == 2
      } else if (strcmp(topic, COMMAND_TOPIC_PUMP_SIRAM.c_str()) == 0) {
        // For the watering pump, command is duration in seconds
        float duration_s = atof(command);
        if (duration_s > 0) {
          control_pump_for_duration(pumps[i], duration_s * 1000);
        }
#endif
      } else {
        // Otherwise, treat the command as a volume in ml
        float volume_ml = atof(command);
        if (volume_ml > 0) {
          control_pump(pumps[i], volume_ml);
        }
      }
      return; // Command handled
    }
  }
}

void actuators_publish_states() {
  LOG_PRINTLN("[Actuators] Publishing initial states to MQTT...");
  for (int i = 0; i < NUM_PUMPS; i++) {
    mqtt_publish_state(pumps[i].stateTopic, pumps[i].isOn ? "ON" : "OFF", true);
  }
#if HYDROPONIC_INSTANCE == 1
  mqtt_publish_state(STATE_TOPIC_SYSTEM_MODE, currentSystemMode == NUTRITION ? "NUTRITION" : "CLEANER", true);
#endif
}

#if HYDROPONIC_INSTANCE == 1
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
  bool shouldAlertBeActive = isnan(values.waterLevelCm) || (values.waterLevelCm <= WATER_LEVEL_CRITICAL_CM);

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
  } else if (!shouldAlertBeActive && isWaterLevelAlertActive) {
    isWaterLevelAlertActive = false;
    LOG_PRINTLN(">>> INFO: Water level has returned to normal. <<<");
    mqtt_publish_alert("OK: Water level is normal.");
  }

  digitalWrite(BUZZER_PIN, isWaterLevelAlertActive ? HIGH : LOW);
}

void actuators_auto_dose_nutrients(const SensorValues& values) {
  unsigned long currentTime = millis();

  if (currentSystemMode != NUTRITION) return;
  if (currentTime - lastAutoDoseCheckTime < AUTO_DOSING_CHECK_INTERVAL_MS) return;
  
  lastAutoDoseCheckTime = currentTime;

  if (are_any_pumps_running() || autoDoseState != IDLE) {
    LOG_PRINTLN("[Auto-Dose] Skipping check: a pump is running or a sequence is in progress.");
    return;
  }

  if (isnan(values.tdsPpm)) {
    LOG_PRINTLN("[Auto-Dose] Skipping check: TDS value is invalid (NAN).");
    return;
  }

  if (values.tdsPpm < TDS_LOWER_THRESHOLD_PPM) {
    LOG_PRINTF("[Auto-Dose] TDS is low (%.1f ppm). Starting nutrient dosing sequence.\n", values.tdsPpm);
    autoDoseState = WAITING_FOR_B_DOSE;
    control_pump(pumps[0], DOSING_AMOUNT_ML); // Start with Nutrient A
  } else {
    LOG_PRINTF("[Auto-Dose] TDS level is OK (%.1f ppm).\n", values.tdsPpm);
  }
}
#endif

// --- Static (Private) Function Implementations ---

static bool are_any_pumps_running() {
  for (int i = 0; i < NUM_PUMPS; i++) {
    if (pumps[i].isOn) return true;
  }
  return false;
}

static void control_pump(Pump& pump, float volume_ml) {
  if (volume_ml <= 0) return;
  if (are_any_pumps_running()) {
    LOG_PRINTF("[Pump] Cannot start %s, another pump is running.\n", pump.name);
    return;
  }
  unsigned long duration_ms = volume_ml * PUMP_MS_PER_ML;
  control_pump_for_duration(pump, duration_ms);
}

static void control_pump_for_duration(Pump& pump, unsigned long duration_ms) {
  if (duration_ms <= 0) return;
  if (are_any_pumps_running()) {
    LOG_PRINTF("[Pump] Cannot start %s, another pump is running.\n", pump.name);
    return;
  }
  LOG_PRINTF("[Pump] Running %s for %lu ms.\n", pump.name, duration_ms);
  pump.stopTime = millis() + duration_ms;
  digitalWrite(pump.pin, HIGH);
  pump.isOn = true;
  mqtt_publish_state(pump.stateTopic, "ON", true);
}
