// =======================================================================
//                           LIBRARY IMPORTS
// =======================================================================
#include "config.h"
#include <Arduino.h>
#include <WiFi.h>         // Untuk koneksi Wi-Fi
#include <PubSubClient.h> // Untuk MQTT

// Custom modules for handling specific logic
#include "sensors.h"
#include "mqtt_handler.h"
#include "actuators.h"

// =======================================================================
//                           GLOBAL VARIABLES & OBJECTS
// =======================================================================
SensorValues currentSensorValues; // A single struct to hold all sensor readings

// Variabel untuk waktu terakhir publikasi/aksi
unsigned long lastSensorPublishTime = 0;
unsigned long lastHeartbeatTime = 0;

// =======================================================================
//                           FUNCTION DECLARATIONS
// =======================================================================
void setupWifi();

// =======================================================================
//                           FUNCTION IMPLEMENTATIONS
// =======================================================================

// Fungsi untuk koneksi Wi-Fi
void setupWifi()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    return; // Sudah terhubung, tidak perlu konek lagi
  }
  Serial.print("\nConnecting to WiFi ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < MAX_RECONNECT_ATTEMPTS)
  {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println(
        "\nFailed to connect to WiFi after " + String(MAX_RECONNECT_ATTEMPTS) +
        " attempts. Restarting ESP32 in 5 seconds...");
    delay(5000);
    ESP.restart(); // Restart ESP32 jika gagal konek WiFi
  }
}

// =======================================================================
//                           SETUP FUNCTION
// =======================================================================
void setup()
{
  Serial.begin(115200);
  Serial.println("--- ESP32 Hydroponic Automation System ---");
  Serial.println("Inisialisasi sistem...");

  // Initialize all hardware modules
  sensors_init();
  actuators_init();
  setupWifi();
  mqtt_init();

  Serial.println("Setup Complete. Memulai Loop...");
}

// =======================================================================
//                           LOOP FUNCTION
// =======================================================================
void loop()
{
  unsigned long currentTime = millis(); // Ambil waktu saat ini

  // Cek koneksi Wi-Fi dan sambungkan kembali jika putus
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("[WiFi] Disconnected! Attempting to reconnect...");
    setupWifi();
  }

  // Cek koneksi MQTT dan sambungkan kembali jika putus (hanya jika WiFi terhubung)
  mqtt_loop(); // Handles MQTT connection and processes incoming messages

  // Handle timed actuator logic (e.g., stopping pumps after a set duration)
  actuators_loop();

  // Baca dan publikasikan data sensor secara berkala
  if (currentTime - lastSensorPublishTime >= SENSOR_PUBLISH_INTERVAL_MS)
  {
    lastSensorPublishTime = currentTime;

    // 1. Baca semua sensor dan simpan nilainya di struct global
    sensors_read_all(currentSensorValues);

    // 2. Publikasikan data sensor ke MQTT (jika terhubung)
    mqtt_publish_sensor_data(currentSensorValues);

    // Perbarui status buzzer berdasarkan nilai sensor yang diukur
    actuators_update_alert_status(currentSensorValues);
  }

  // Publikasikan heartbeat secara berkala (hanya jika terhubung MQTT)
  if (currentTime - lastHeartbeatTime >= HEARTBEAT_INTERVAL_MS)
  {
    lastHeartbeatTime = currentTime;
    if (mqtt_is_connected())
      mqtt_publish_heartbeat();
    else
    {
      Serial.println("[Heartbeat] Skipping heartbeat: MQTT not connected.");
    }
  }

  delay(50); // Delay singkat untuk stabilitas
}
