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

// --- Pin Definitions ---
// Sensor Pins
const int ULTRASONIC_TRIGGER_PIN = 5;  // JSN-SR04T Trig
const int ULTRASONIC_ECHO_PIN = 4;   // JSN-SR04T Echo
const int ONE_WIRE_BUS = 18;         // DS18B20 Data
const int DHT_PIN = 13;              // DHT22 Data
const int TDS_SENSOR_PIN = 34;       // TDS Sensor Analog Out
const int PH_SENSOR_PIN = 32;        // pH Sensor Analog Out (GPIO 32 - sesuai kalibrasi)
const int PZEM_RX_PIN = 16;          // ESP32 RX2, connects to PZEM TX
const int PZEM_TX_PIN = 17;          // ESP32 TX2, connects to PZEM RX

// Actuator Pins
const int BUZZER_PIN = 19;           // Buzzer control
const int PUMP_NUTRISI_A_PIN = 25;   // Relay IN1
const int PUMP_NUTRISI_B_PIN = 26;   // Relay IN2
const int PUMP_PH_PIN = 27;          // Relay IN3
const int PUMP_SIRAM_PIN = 33;       // Relay IN4
const int PUMP_TANDON_PIN = 15;      // Relay IN5 (untuk katup solenoid/pompa pengisi)

// --- Hardware & System Parameters ---
const int TANDON_MAX_HEIGHT_CM = 100;
const int ULTRASONIC_MAX_DISTANCE_CM = 400;
const int DHT_TYPE = DHT22;
const float WATER_LEVEL_CRITICAL_CM = 20.0;

// --- Sensor Calibration ---
const float TDS_K_VALUE = 635.40;
const float TDS_TEMP_COEFF = 0.02;

// ===================================================================
// == KALIBRASI SENSOR pH - DATA AKTUAL DARI EKSPERIMEN 4-TITIK ==
// ===================================================================
// Hasil kalibrasi aktual menggunakan buffer pH 4.01, 6.86, 7.0, dan 9.18
// Data ini sudah diverifikasi dan memberikan akurasi maksimal
//
// CATATAN: Jangan ubah nilai ini kecuali melakukan kalibrasi ulang
const float PH_CALIBRATION_VOLTAGE_401 = 3.045; // Tegangan untuk pH 4.01 (dari kalibrasi aktual)
const float PH_CALIBRATION_VOLTAGE_686 = 2.510; // Tegangan untuk pH 6.86 (dari eksperimen aktual)
const float PH_CALIBRATION_VOLTAGE_7   = 2.814; // Tegangan untuk pH 7.0 (dari kalibrasi aktual)
const float PH_CALIBRATION_VOLTAGE_918 = 2.025; // Tegangan untuk pH 9.18 (dari eksperimen aktual)

// --- Timing & Network ---
const IPAddress PRIMARY_DNS(8, 8, 8, 8);
const long SENSOR_PUBLISH_INTERVAL_MS = 5000;   // 5 seconds
const long HEARTBEAT_INTERVAL_MS = 10000;  // 10 seconds
const long WIFI_RECONNECT_DELAY_MS = 5000;
const long MQTT_RECONNECT_DELAY_MS = 5000; 
const int MAX_RECONNECT_ATTEMPTS = 60;

// --- Calculated Constants ---
// Pump calibration: (seconds per revolution * 60 * 1000) / ml per revolution
// Example: (13 sec/rev * 60 sec/min * 1000 ms/sec) / 200 ml/rev = 3900 ms/ml
const float PUMP_MS_PER_ML = (13.0 * 60.0 * 1000.0) / 200.0;


// =======================================================================
//                           MQTT TOPIC DEFINITIONS
// =======================================================================
// Topics are constructed dynamically from the BASE_TOPIC defined above
const std::string STATE_TOPIC_LEVEL = std::string(BASE_TOPIC) + "/air/level_cm";
const std::string STATE_TOPIC_DISTANCE = std::string(BASE_TOPIC) + "/air/jarak_sensor_cm";
const std::string STATE_TOPIC_WATER_TEMPERATURE = std::string(BASE_TOPIC) + "/air/suhu_c";
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
