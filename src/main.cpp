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
// Struct tunggal untuk menampung semua pembacaan sensor
// A single struct to hold all sensor readings
SensorValues currentSensorValues;

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

/**
 * @brief Fungsi untuk koneksi Wi-Fi.
 *        Mencoba terhubung ke WiFi dan akan me-restart ESP32 jika gagal setelah beberapa kali percobaan.
 * 
 * @brief Function for Wi-Fi connection.
 *        Attempts to connect to WiFi and will restart the ESP32 if it fails after several attempts.
 */
void setupWifi()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    return; // Sudah terhubung, tidak perlu konek lagi / Already connected, no need to reconnect
  }
  LOG_PRINT("\nConnecting to WiFi ");
  LOG_PRINT(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < MAX_RECONNECT_ATTEMPTS) {
    delay(500);
    LOG_PRINT(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    LOG_PRINTLN("\nWiFi Connected!");
    LOG_PRINT("IP Address: ");
    LOG_PRINTLN(WiFi.localIP());
  } else {
    LOG_PRINTLN(
        "\nFailed to connect to WiFi after " + String(MAX_RECONNECT_ATTEMPTS) +
        " attempts. Restarting ESP32 in 5 seconds...");
    delay(5000);
    ESP.restart(); // Restart ESP32 jika gagal konek WiFi / Restart ESP32 if WiFi connection fails
  }
}

// =======================================================================
//                           SETUP FUNCTION
// =======================================================================
/**
 * @brief Fungsi setup utama. Dijalankan sekali saat ESP32 boot.
 *        Menginisialisasi Serial, sensor, aktuator, WiFi, dan MQTT.
 * 
 * @brief Main setup function. Runs once when the ESP32 boots.
 *        Initializes Serial, sensors, actuators, WiFi, and MQTT.
 */
void setup()
{
  Serial.begin(115200);
  LOG_PRINTLN("--- ESP32 Hydroponic Automation System ---");
  LOG_PRINTLN("Inisialisasi sistem... / System Initialization...");

  // Initialize all hardware modules
  // Inisialisasi semua modul perangkat keras
  sensors_init();
  actuators_init();
  setupWifi();
  mqtt_init();

  LOG_PRINTLN("Setup Complete. Memulai Loop... / Setup Complete. Starting Loop...");
}

// =======================================================================
//                           LOOP FUNCTION
// =======================================================================
/**
 * @brief Fungsi loop utama. Dijalankan berulang kali setelah setup().
 *        Bertanggung jawab untuk menjaga koneksi, membaca sensor, dan mengirim data.
 * 
 * @brief Main loop function. Runs repeatedly after setup().
 *        Responsible for maintaining connections, reading sensors, and sending data.
 */
void loop()
{
  unsigned long currentTime = millis(); // Ambil waktu saat ini / Get the current time

  // Cek koneksi Wi-Fi dan sambungkan kembali jika putus
  // Check Wi-Fi connection and reconnect if lost
  if (WiFi.status() != WL_CONNECTED) {
    LOG_PRINTLN("[WiFi] Disconnected! Attempting to reconnect...");
    setupWifi();
  }

  // Cek koneksi MQTT dan sambungkan kembali jika putus (hanya jika WiFi terhubung)
  // Check MQTT connection and reconnect if lost (only if WiFi is connected)
  mqtt_loop(); // Handles MQTT connection and processes incoming messages

  // Handle timed actuator logic (e.g., stopping pumps after a set duration)
  // Menangani logika aktuator berwaktu (misalnya, menghentikan pompa setelah durasi tertentu)
  actuators_loop();

  // Baca dan publikasikan data sensor secara berkala
  // Read and publish sensor data periodically
  if (currentTime - lastSensorPublishTime >= SENSOR_PUBLISH_INTERVAL_MS) {
    lastSensorPublishTime = currentTime;

    // 1. Baca semua sensor dan simpan nilainya di struct global
    // 1. Read all sensors and store their values in the global struct
    sensors_read_all(currentSensorValues);

    // 2. Publikasikan data sensor ke MQTT (jika terhubung)
    // 2. Publish sensor data to MQTT (if connected)
    mqtt_publish_sensor_data(currentSensorValues);

    // Perbarui status buzzer berdasarkan nilai sensor yang diukur
    // Update the buzzer status based on the measured sensor values
    actuators_update_alert_status(currentSensorValues);

    // 3. Periksa apakah dosis otomatis diperlukan berdasarkan nilai sensor
    // 3. Check if auto-dosing is required based on sensor values
    actuators_auto_dose_nutrients(currentSensorValues);
  }

  // Publikasikan heartbeat secara berkala (hanya jika terhubung MQTT)
  // Publish heartbeat periodically (only if connected to MQTT)
  if (currentTime - lastHeartbeatTime >= HEARTBEAT_INTERVAL_MS) {
    lastHeartbeatTime = currentTime;
    if (mqtt_is_connected())
      mqtt_publish_heartbeat();
    else {
      LOG_PRINTLN("[Heartbeat] Skipping heartbeat: MQTT not connected.");
    }
  }

  delay(1); // Delay singkat untuk stabilitas dan yield ke RTOS / Short delay for stability and to yield to the RTOS
}
