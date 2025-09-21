/**
 * @file config.cpp
 * @brief Defines and initializes all global configuration variables for the system.
 */

#include "config.h"
#include <DHT.h> // Include the DHT library to define DHT22

// =======================================================================
//                           WIFI & MQTT CREDENTIALS
// =======================================================================
// These values are injected by platformio.ini via build_flags from the
// credentials.ini file. This keeps sensitive data out of the source code.
const char *WIFI_SSID = ENV_WIFI_SSID;
const char *WIFI_PASSWORD = ENV_WIFI_PASSWORD;
const char *MQTT_USERNAME = ENV_MQTT_USER;
const char *MQTT_PASSWORD = ENV_MQTT_PASS;
const char *MQTT_SERVER = ENV_MQTT_SERVER;
const int MQTT_PORT = ENV_MQTT_PORT;


// =======================================================================
//                       SYSTEM-WIDE CONSTANTS
// =======================================================================

// --- MQTT Identity ---
const char *MQTT_CLIENT_ID = "esp32-hydroponic-" STR(HYDROPONIC_INSTANCE_ID);
const char *BASE_TOPIC = "hidroponik/" STR(HYDROPONIC_INSTANCE_ID);

// --- Hardware & System Parameters ---
const int TANDON_MAX_HEIGHT_CM = 100;
const int ULTRASONIC_MAX_DISTANCE_CM = 400;
const int DHT_TYPE = DHT22;
const float WATER_LEVEL_CRITICAL_CM = 20.0;

// --- Sensor Calibration ---
const float TDS_K_VALUE = 635.40;
const float TDS_TEMP_COEFF = 0.02;

// ===================================================================
// == PENTING: KALIBRASI SENSOR pH WAJIB DILAKUKAN ==
// ===================================================================
// Nilai di bawah ini diambil dari skrip kalibrasi 4-titik Anda
// di `calibration/src/main.cpp` untuk akurasi maksimal.
const float PH_CALIBRATION_VOLTAGE_401 = 3.045; // Tegangan untuk pH 4.01
const float PH_CALIBRATION_VOLTAGE_686 = 2.510; // Tegangan untuk pH 6.86
const float PH_CALIBRATION_VOLTAGE_918 = 2.025; // Tegangan untuk pH 9.18

// --- Timing & Network ---
const IPAddress PRIMARY_DNS(8, 8, 8, 8);
const long SENSOR_PUBLISH_INTERVAL_MS = 5000;   // 5 seconds
const long HEARTBEAT_INTERVAL_MS = 10000;  // 10 seconds
const long ACTUATOR_STATE_PUBLISH_INTERVAL_MS = 5000;  // 5 seconds
const long AUTOMATION_STATE_PUBLISH_INTERVAL_MS = 5000;  // 5 seconds
const long WIFI_RECONNECT_DELAY_MS = 5000;
const long MQTT_RECONNECT_DELAY_MS = 5000; 
const int MAX_RECONNECT_ATTEMPTS = 60;

// --- Calculated Constants & Calibration ---
// ===================================================================
// == PENTING: KALIBRASI POMPA BARU - UPDATED SEPTEMBER 2025 ==
// ===================================================================
// Data kalibrasi pompa nutrisi yang telah diperbarui di lapangan:
// - 100 ml dalam 3 detik = 33.33 ml/detik
// - 1 liter dalam 30 detik = 33.33 ml/detik (verification)
// - Kalkulasi: 3000 ms / 100 ml = 30 ms/ml
const float PUMP_MS_PER_ML = 30.0;


// =======================================================================
//                           MQTT TOPIC DEFINITIONS
// =======================================================================
// Topics are constructed dynamically from the BASE_TOPIC defined above
const std::string STATE_TOPIC_LEVEL = std::string(BASE_TOPIC) + "/air/level_cm";
const std::string STATE_TOPIC_DISTANCE = std::string(BASE_TOPIC) + "/air/distance_cm";
const std::string STATE_TOPIC_WATER_TEMPERATURE = std::string(BASE_TOPIC) + "/air/water_temp_c";
const std::string STATE_TOPIC_AIR_TEMPERATURE = std::string(BASE_TOPIC) + "/udara/suhu_c";
const std::string STATE_TOPIC_HUMIDITY = std::string(BASE_TOPIC) + "/udara/kelembaban_persen";
const std::string STATE_TOPIC_TDS = std::string(BASE_TOPIC) + "/air/tds_ppm";
const std::string STATE_TOPIC_PH = std::string(BASE_TOPIC) + "/air/ph";
const std::string STATE_TOPIC_VOLTAGE = std::string(BASE_TOPIC) + "/listrik/tegangan_v";
const std::string STATE_TOPIC_CURRENT = std::string(BASE_TOPIC) + "/listrik/arus_a";
const std::string STATE_TOPIC_POWER = std::string(BASE_TOPIC) + "/listrik/daya_w";
const std::string STATE_TOPIC_ENERGY = std::string(BASE_TOPIC) + "/listrik/energi_kwh";
const std::string STATE_TOPIC_FREQUENCY = std::string(BASE_TOPIC) + "/listrik/frekuensi_hz";
const std::string STATE_TOPIC_PF = std::string(BASE_TOPIC) + "/listrik/power_factor";
const std::string AVAILABILITY_TOPIC = std::string(BASE_TOPIC) + "/status/LWT";
const std::string HEARTBEAT_TOPIC = std::string(BASE_TOPIC) + "/status/HEARTBEAT";
const std::string MQTT_GLOBAL_ALERT_TOPIC = std::string(BASE_TOPIC) + "/peringatan";

const std::string COMMAND_TOPIC_SYSTEM_MODE = std::string(BASE_TOPIC) + "/sistem/mode/kontrol";
const std::string STATE_TOPIC_SYSTEM_MODE = std::string(BASE_TOPIC) + "/sistem/mode/status";
const std::string COMMAND_TOPIC_PUMP_A = std::string(BASE_TOPIC) + "/pompa/nutrisi_a/kontrol";
const std::string STATE_TOPIC_PUMP_A = std::string(BASE_TOPIC) + "/pompa/nutrisi_a/status";
const std::string COMMAND_TOPIC_PUMP_B = std::string(BASE_TOPIC) + "/pompa/nutrisi_b/kontrol";
const std::string STATE_TOPIC_PUMP_B = std::string(BASE_TOPIC) + "/pompa/nutrisi_b/status";
const std::string COMMAND_TOPIC_PUMP_PH = std::string(BASE_TOPIC) + "/pompa/ph/kontrol";
const std::string STATE_TOPIC_PUMP_PH = std::string(BASE_TOPIC) + "/pompa/ph/status";
const std::string COMMAND_TOPIC_PUMP_SIRAM = std::string(BASE_TOPIC) + "/pompa/penyiraman/kontrol";
const std::string STATE_TOPIC_PUMP_SIRAM = std::string(BASE_TOPIC) + "/pompa/penyiraman/status";
const std::string COMMAND_TOPIC_PUMP_TANDON = std::string(BASE_TOPIC) + "/pompa/tandon/kontrol";
const std::string STATE_TOPIC_PUMP_TANDON = std::string(BASE_TOPIC) + "/pompa/tandon/status";

// Automation Topics
const std::string COMMAND_TOPIC_AUTO_DOSING = std::string(BASE_TOPIC) + "/automasi/dosing/kontrol";
const std::string STATE_TOPIC_AUTO_DOSING = std::string(BASE_TOPIC) + "/automasi/dosing/status";
const std::string COMMAND_TOPIC_AUTO_REFILL = std::string(BASE_TOPIC) + "/automasi/refill/kontrol";
const std::string STATE_TOPIC_AUTO_REFILL = std::string(BASE_TOPIC) + "/automasi/refill/status";
const std::string COMMAND_TOPIC_AUTO_IRRIGATION = std::string(BASE_TOPIC) + "/automasi/irrigation/kontrol";
const std::string STATE_TOPIC_AUTO_IRRIGATION = std::string(BASE_TOPIC) + "/automasi/irrigation/status";
