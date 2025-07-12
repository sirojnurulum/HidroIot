#include "actuators.h"
#include "config.h"
#include "mqtt_handler.h" // For publishing alerts and state

// --- Module-Private (Static) Variables ---
// These variables are now local to this file and hidden from other modules.
static bool isPumpAOn = false;
static bool isPumpBOn = false;
static bool isPumpPHOn = false;

// This variable is also moved here from main.cpp
static bool isWaterLevelAlertActive = false;

// Variables to track when each pump should stop. 0 means it's not on a timed run.
static unsigned long pumpA_stop_time = 0;
static unsigned long pumpB_stop_time = 0;
static unsigned long pumpPH_stop_time = 0;

void actuators_init() {
  Serial.println("Initializing actuators...");
  // Initialize pump pins as OUTPUT and ensure they are OFF at startup
  pinMode(PUMP_NUTRISI_A_PIN, OUTPUT);
  pinMode(PUMP_NUTRISI_B_PIN, OUTPUT);
  pinMode(PUMP_PH_PIN, OUTPUT);
  digitalWrite(PUMP_NUTRISI_A_PIN, LOW); // LOW = OFF
  digitalWrite(PUMP_NUTRISI_B_PIN, LOW);
  digitalWrite(PUMP_PH_PIN, LOW);

  // Initialize buzzer pin and ensure it's OFF
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
}

void actuators_loop() {
  unsigned long currentTime = millis();

  // Check and stop Pump A if its time is up
  if (isPumpAOn && pumpA_stop_time > 0 && currentTime >= pumpA_stop_time) {
    digitalWrite(PUMP_NUTRISI_A_PIN, LOW);
    isPumpAOn = false;
    pumpA_stop_time = 0; // Clear the stop time
    Serial.println("[Pump Control] Pump Nutrisi A finished pumping.");
    mqtt_publish_state(STATE_TOPIC_PUMP_A, "OFF", true);
  }

  // Check and stop Pump B if its time is up
  if (isPumpBOn && pumpB_stop_time > 0 && currentTime >= pumpB_stop_time) {
    digitalWrite(PUMP_NUTRISI_B_PIN, LOW);
    isPumpBOn = false;
    pumpB_stop_time = 0;
    Serial.println("[Pump Control] Pump Nutrisi B finished pumping.");
    mqtt_publish_state(STATE_TOPIC_PUMP_B, "OFF", true);
  }

  // Check and stop Pump PH if its time is up
  if (isPumpPHOn && pumpPH_stop_time > 0 && currentTime >= pumpPH_stop_time) {
    digitalWrite(PUMP_PH_PIN, LOW);
    isPumpPHOn = false;
    pumpPH_stop_time = 0;
    Serial.println("[Pump Control] Pump pH finished pumping.");
    mqtt_publish_state(STATE_TOPIC_PUMP_PH, "OFF", true);
  }
}

void actuators_handle_pump_command(const char* topic, const String& command) {
  int pumpPin;
  bool* pumpStatus;
  unsigned long* stopTime;
  const char* stateTopic;
  const char* pumpName;

  if (strcmp(topic, COMMAND_TOPIC_PUMP_A) == 0) {
    pumpPin = PUMP_NUTRISI_A_PIN;
    pumpStatus = &isPumpAOn;
    stopTime = &pumpA_stop_time;
    stateTopic = STATE_TOPIC_PUMP_A;
    pumpName = "Nutrisi A";
  } else if (strcmp(topic, COMMAND_TOPIC_PUMP_B) == 0) {
    pumpPin = PUMP_NUTRISI_B_PIN;
    pumpStatus = &isPumpBOn;
    stopTime = &pumpB_stop_time;
    stateTopic = STATE_TOPIC_PUMP_B;
    pumpName = "Nutrisi B";
  } else if (strcmp(topic, COMMAND_TOPIC_PUMP_PH) == 0) {
    pumpPin = PUMP_PH_PIN;
    pumpStatus = &isPumpPHOn;
    stopTime = &pumpPH_stop_time;
    stateTopic = STATE_TOPIC_PUMP_PH;
    pumpName = "pH";
  } else {
    return; // Not a pump topic we handle
  }

  // Handle emergency stop command
  if (command == "OFF") {
    digitalWrite(pumpPin, LOW);
    *pumpStatus = false;
    *stopTime = 0; // Cancel any timed run
    Serial.printf("[Pump Control] Pump %s force stopped.\n", pumpName);
    mqtt_publish_state(stateTopic, "OFF", true);
    return;
  }

  // Otherwise, treat the command as a volume in ml
  float volume_ml = command.toFloat();
  if (volume_ml > 0) {
    unsigned long duration_ms = volume_ml * PUMP_MS_PER_ML;
    Serial.printf("[Pump Control] Pumping %.1f ml from %s for %lu ms.\n", volume_ml, pumpName, duration_ms);
    *stopTime = millis() + duration_ms;
    digitalWrite(pumpPin, HIGH);
    *pumpStatus = true;
    mqtt_publish_state(stateTopic, "ON", true);
  }
}

void actuators_update_alert_status(const SensorValues& values) {
  // This logic is moved directly from main.cpp's updateBuzzerAlert()
  bool shouldAlertBeActive = false;
  if (isnan(values.waterLevelCm)) {
    shouldAlertBeActive = true;
  } else if (values.waterLevelCm <= WATER_LEVEL_CRITICAL_CM) {
    shouldAlertBeActive = true;
  }

  // Send notification only when the alert status changes
  if (shouldAlertBeActive && !isWaterLevelAlertActive) {
    isWaterLevelAlertActive = true;
    char alertMessage[60];
    if (isnan(values.waterLevelCm)) {
      strcpy(alertMessage, "ALERT: Level air tidak terdeteksi atau di luar jangkauan!");
    } else {
      sprintf(alertMessage, "ALERT: Level air kritis! %.1f cm", values.waterLevelCm);
    }
    mqtt_publish_alert(alertMessage);
    Serial.printf(">>> Peringatan: %s <<<\n", alertMessage);
  } else if (!shouldAlertBeActive && isWaterLevelAlertActive) {
    isWaterLevelAlertActive = false;
    mqtt_publish_alert("Level air normal kembali.");
    Serial.println("Level air kembali normal.");
  }

  // Control the physical buzzer based on the alert status
  digitalWrite(BUZZER_PIN, isWaterLevelAlertActive ? HIGH : LOW);
}

void actuators_publish_states() {
  Serial.println("[MQTT] Publishing initial actuator states...");
  mqtt_publish_state(STATE_TOPIC_PUMP_A, isPumpAOn ? "ON" : "OFF", true);
  mqtt_publish_state(STATE_TOPIC_PUMP_B, isPumpBOn ? "ON" : "OFF", true);
  mqtt_publish_state(STATE_TOPIC_PUMP_PH, isPumpPHOn ? "ON" : "OFF", true);
}