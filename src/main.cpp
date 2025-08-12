/**
 * @file main.cpp
 * @brief Main entry point for the ESP32 Hydroponics Automation System.
 *
 * This file contains the primary setup() and loop() functions that orchestrate
 * the initialization and continuous operation of all system modules, including
 * sensors, actuators, and MQTT communication.
 */

#include "config.h"
#include <WiFi.h>
#include "sensors.h"
#include "mqtt_handler.h"
#include "actuators.h"

// --- Global Variables ---

/// @brief A global struct to hold the most recent sensor readings.
static SensorValues currentSensorValues;
/// @brief Tracks the last time sensor data was published to MQTT to manage timing.
static unsigned long lastSensorPublishTime = 0;
/// @brief Tracks the last time a heartbeat was sent to MQTT.
static unsigned long lastHeartbeatTime = 0;
/// @brief Tracks the last time actuator states were published to MQTT.
static unsigned long lastActuatorStatePublishTime = 0;
/// @brief Tracks the last time automation states were published to MQTT.
static unsigned long lastAutomationStatePublishTime = 0;

// --- Forward Declarations ---
static void connectToWifi();

/**
 * @brief The main setup function, run once on boot.
 * Initializes the serial monitor, all hardware modules (sensors, actuators),
 * connects to WiFi, and initializes the MQTT client.
 */
void setup() {
  Serial.begin(115200);
  // A small delay to allow the serial monitor to connect.
  delay(1000); 
  LOG_PRINTLN("\n--- ESP32 Hydroponic System Initializing ---");

  sensors_init();
  actuators_init();
  connectToWifi();
  mqtt_init();

  LOG_PRINTLN("\n--- System Initialization Complete. Starting main loop. ---\n");
}

/**
 * @brief The main loop, run repeatedly after setup.
 * Manages WiFi and MQTT connections, runs the loop functions for actuators
 * and MQTT, and triggers timed actions like reading sensors and sending heartbeats.
 */
void loop() {
  unsigned long currentTime = millis();

  // Ensure WiFi is connected before proceeding.
  if (WiFi.status() != WL_CONNECTED) {
    connectToWifi();
  }

  // Run the loop functions for each module. These are non-blocking.
  mqtt_loop();
  actuators_loop(currentSensorValues);

  // --- Timed Actions using a non-blocking approach ---

  // Periodically read sensors and publish the data.
  if (currentTime - lastSensorPublishTime >= SENSOR_PUBLISH_INTERVAL_MS) {
    lastSensorPublishTime = currentTime;

    sensors_read_all(currentSensorValues);
    mqtt_publish_sensor_data(currentSensorValues);
    actuators_update_alert_status(currentSensorValues);
  }

  // Periodically send a heartbeat to show the device is alive.
  if (currentTime - lastHeartbeatTime >= HEARTBEAT_INTERVAL_MS) {
    lastHeartbeatTime = currentTime;
    mqtt_publish_heartbeat();
  }

  // Periodically publish actuator states to keep Home Assistant synchronized.
  if (currentTime - lastActuatorStatePublishTime >= ACTUATOR_STATE_PUBLISH_INTERVAL_MS) {
    lastActuatorStatePublishTime = currentTime;
    actuators_publish_states();
  }

  // Periodically publish automation states to keep Home Assistant synchronized.
  if (currentTime - lastAutomationStatePublishTime >= AUTOMATION_STATE_PUBLISH_INTERVAL_MS) {
    lastAutomationStatePublishTime = currentTime;
    actuators_publish_automation_states();
  }
}

/**
 * @brief Connects to the configured WiFi network.
 * This is a blocking function that will attempt to connect for a configured
 * number of retries. If it fails to connect, it will restart the ESP32
 * to attempt a full re-initialization.
 */
static void connectToWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  LOG_PRINTF("\n[WiFi] Connecting to SSID: %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA); // Set WiFi to station mode
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < MAX_RECONNECT_ATTEMPTS) {
    LOG_PRINT(".");
    // This delay is acceptable here as it only occurs during connection setup.
    delay(WIFI_RECONNECT_DELAY_MS);
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    LOG_PRINTLN("\n[WiFi] Connected!");
    LOG_PRINTF("[WiFi] IP Address: %s\n", WiFi.localIP().toString().c_str());
  } else {
    LOG_PRINTLN("\n[WiFi] Failed to connect after multiple attempts. Restarting...");
    delay(1000); // Short delay to allow the log message to be sent.
    ESP.restart();
  }
}
