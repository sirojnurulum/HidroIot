// =======================================================================
//                           LIBRARY IMPORTS
// =======================================================================
#include "config.h"
#include <Arduino.h>
#include <WiFi.h>         // Untuk koneksi Wi-Fi

// Custom modules for handling specific logic
// Modul kustom untuk menangani logika spesifik
#include "sensors.h"
#include "mqtt_handler.h"
#include "actuators.h"

// =======================================================================
//                           GLOBAL VARIABLES & OBJECTS
// =======================================================================
#if HYDROPONIC_INSTANCE == 1
// Struct tunggal untuk menampung semua pembacaan sensor
SensorValues currentSensorValues;
#endif

// Variabel untuk waktu terakhir publikasi/aksi
// Variables for the last time of publication/action
unsigned long lastSensorPublishTime = 0;
unsigned long lastHeartbeatTime = 0;

// =======================================================================
//                           FUNCTION DECLARATIONS
// =======================================================================
void setupWifi();

// =======================================================================
//                           FUNCTION IMPLEMENTATIONS
// =======================================================================
void setupWifi()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    return;
  }
  LOG_PRINTF("\n[WiFi] Attempting to connect to SSID: '%s'\n", WIFI_SSID);

  // Memulai koneksi WiFi dengan DHCP
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < MAX_RECONNECT_ATTEMPTS) {
    LOG_PRINTF("[WiFi] Connection attempt %d... Status: %d\n", attempts + 1, WiFi.status());
    delay(1000);
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    LOG_PRINTLN("\nWiFi Connected!");
    LOG_PRINT("IP Address: ");
    LOG_PRINTLN(WiFi.localIP());
  } else {
    LOG_PRINTLN("\nFailed to connect to WiFi. Restarting ESP32...");
    delay(5000);
    ESP.restart();
  }
}

void setup()
{
  Serial.begin(115200);
  LOG_PRINTLN("--- ESP32 Hydroponic Automation System ---");
  LOG_PRINTLN("Inisialisasi sistem... / System Initialization...");

  LOG_PRINTLN("\n--- Verifying Credentials ---");
  LOG_PRINTF("Read WIFI_SSID:     [%s]\n", WIFI_SSID);
  LOG_PRINTF("Read WIFI_PASSWORD: [%s]\n", WIFI_PASSWORD); // Hanya untuk debugging
  LOG_PRINTF("Read MQTT_USER:     [%s]\n", MQTT_USERNAME);
  LOG_PRINTF("Read MQTT_PASS:     [%s]\n", MQTT_PASSWORD); // Hanya untuk debugging
  LOG_PRINTLN("---------------------------\n");

#if HYDROPONIC_INSTANCE == 1
  sensors_init();
#endif
  actuators_init();
  setupWifi();
  mqtt_init();

  LOG_PRINTLN("Setup Complete. Memulai Loop... / Setup Complete. Starting Loop...");
}

void loop()
{
  unsigned long currentTime = millis();

  if (WiFi.status() != WL_CONNECTED) {
    LOG_PRINTLN("[WiFi] Disconnected! Attempting to reconnect...");
    setupWifi();
  }

  mqtt_loop();
  actuators_loop();

#if HYDROPONIC_INSTANCE == 1
  if (currentTime - lastSensorPublishTime >= SENSOR_PUBLISH_INTERVAL_MS) {
    lastSensorPublishTime = currentTime;
    sensors_read_all(currentSensorValues);
    mqtt_publish_sensor_data(currentSensorValues);
    actuators_update_alert_status(currentSensorValues);
    actuators_auto_dose_nutrients(currentSensorValues);
  }
#endif

  if (currentTime - lastHeartbeatTime >= HEARTBEAT_INTERVAL_MS) {
    lastHeartbeatTime = currentTime;
    if (mqtt_is_connected())
      mqtt_publish_heartbeat();
    else {
      LOG_PRINTLN("[Heartbeat] Skipping heartbeat: MQTT not connected.");
    }
  }

  delay(1); // Delay singkat untuk stabilitas dan yield ke RTOS
}
