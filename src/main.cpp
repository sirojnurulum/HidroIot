#include "config.h"
#include <WiFi.h>
#include "sensors.h"
#include "mqtt_handler.h"
#include "actuators.h"

// --- Global Variables ---
#if HYDROPONIC_INSTANCE == 1
static SensorValues currentSensorValues;
#endif

static unsigned long lastSensorPublishTime = 0;
static unsigned long lastHeartbeatTime = 0;

// --- Forward Declarations ---
void connectToWifi();

void setup() {
  Serial.begin(115200);
  LOG_PRINTLN("\n--- ESP32 Hydroponic System Initializing ---");

#if HYDROPONIC_INSTANCE == 1
  sensors_init();
#endif
  actuators_init();
  connectToWifi();
  mqtt_init();

  LOG_PRINTLN("\n--- System Initialization Complete. Starting main loop. ---\n");
}

void loop() {
  unsigned long currentTime = millis();

  // Ensure WiFi is connected before proceeding
  if (WiFi.status() != WL_CONNECTED) {
    connectToWifi();
  }

  // Run the loop functions for each module
  mqtt_loop();
  actuators_loop();

  // --- Timed Actions ---
#if HYDROPONIC_INSTANCE == 1
  // Periodically read sensors and publish the data
  if (currentTime - lastSensorPublishTime >= SENSOR_PUBLISH_INTERVAL_MS) {
    lastSensorPublishTime = currentTime;

    sensors_read_all(currentSensorValues);
    mqtt_publish_sensor_data(currentSensorValues);
    actuators_update_alert_status(currentSensorValues);
    actuators_auto_dose_nutrients(currentSensorValues);
  }
#endif

  // Periodically send a heartbeat to show the device is alive
  if (currentTime - lastHeartbeatTime >= HEARTBEAT_INTERVAL_MS) {
    lastHeartbeatTime = currentTime;
    mqtt_publish_heartbeat();
  }
}

/**
 * @brief Connects to WiFi with a timeout and handles failure by restarting.
 */
void connectToWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  LOG_PRINTF("\n[WiFi] Connecting to SSID: %s\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < MAX_RECONNECT_ATTEMPTS) {
    LOG_PRINT(".");
    delay(WIFI_RECONNECT_DELAY_MS);
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    LOG_PRINTLN("\n[WiFi] Connected!");
    LOG_PRINTF("[WiFi] IP Address: %s\n", WiFi.localIP().toString().c_str());
  } else {
    LOG_PRINTLN("\n[WiFi] Failed to connect after multiple attempts. Restarting...");
    delay(1000);
    ESP.restart();
  }
}
