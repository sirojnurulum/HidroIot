#include "config.h"

#if !defined(HYDROPONIC_INSTANCE)
  #error "HYDROPONIC_INSTANCE is not defined. Please select a build environment in platformio.ini (e.g., 'produksi' or 'penyemaian')."
#endif

#if HYDROPONIC_INSTANCE == 1
// =======================================================================
//           KONFIGURASI UNTUK INSTANCE 'PRODUKSI' (ID: 1)
// =======================================================================

// --- WIFI CONFIGURATION ---
const char *WIFI_SSID = ENV_WIFI_SSID;
const char *WIFI_PASSWORD = ENV_WIFI_PASSWORD;

// --- MQTT CONFIGURATION ---
const char *MQTT_SERVER = "ha.o-m-b.id";
const int MQTT_PORT = 1883;
const char *MQTT_USERNAME = ENV_MQTT_USER;
const char *MQTT_PASSWORD = ENV_MQTT_PASS;
const char *MQTT_CLIENT_ID = "esp32hidro_produksi";
const char *BASE_TOPIC = "hidroponik/produksi";

// --- PIN DEFINITIONS for 'produksi' ---
const int ULTRASONIC_TRIGGER_PIN = 5;
const int ULTRASONIC_ECHO_PIN = 4;
const int ONE_WIRE_BUS = 16;
const int DHT_PIN = 13;
const int TDS_SENSOR_PIN = 34;
const int BUZZER_PIN = 17;
const int PUMP_NUTRISI_A_PIN = 25;
const int PUMP_NUTRISI_B_PIN = 26;
const int PUMP_PH_PIN = 27;

// --- SYSTEM CONFIG for 'produksi' ---
const float WATER_LEVEL_CRITICAL_CM = 20.0;
const float TDS_LOWER_THRESHOLD_PPM = 650.0;
const float DOSING_AMOUNT_ML = 20.0;
const long AUTO_DOSING_CHECK_INTERVAL_MS = 1800000; // 30 menit
const float TDS_K_VALUE = 635.40;
const float TDS_TEMP_COEFF = 0.02;

#elif HYDROPONIC_INSTANCE == 2
// =======================================================================
//          KONFIGURASI UNTUK INSTANCE 'PENYEMAIAN' (ID: 2)
// =======================================================================

// --- WIFI CONFIGURATION ---
const char *WIFI_SSID = ENV_WIFI_SSID;
const char *WIFI_PASSWORD = ENV_WIFI_PASSWORD;

// --- MQTT CONFIGURATION ---
const char *MQTT_SERVER = "ha.o-m-b.id";
const int MQTT_PORT = 1883;
const char *MQTT_USERNAME = ENV_MQTT_USER;
const char *MQTT_PASSWORD = ENV_MQTT_PASS;
const char *MQTT_CLIENT_ID = "esp32hidro_penyemaian";
const char *BASE_TOPIC = "hidroponik/penyemaian";

// --- PIN DEFINITIONS for 'penyemaian' (Watering Only) ---
const int PUMP_SIRAM_PIN = 32;

#endif

// =======================================================================
//                           MQTT TOPIC DEFINITIONS
// =======================================================================
const std::string STATE_TOPIC_LEVEL = std::string(BASE_TOPIC) + "/air/level_cm";
const std::string STATE_TOPIC_DISTANCE = std::string(BASE_TOPIC) + "/air/jarak_sensor_cm";
const std::string STATE_TOPIC_WATER_TEMPERATURE = std::string(BASE_TOPIC) + "/air/suhu_c";
const std::string STATE_TOPIC_AIR_TEMPERATURE = std::string(BASE_TOPIC) + "/udara/suhu_c";
const std::string STATE_TOPIC_HUMIDITY = std::string(BASE_TOPIC) + "/udara/kelembaban_persen";
const std::string STATE_TOPIC_TDS = std::string(BASE_TOPIC) + "/air/tds_ppm";
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
const std::string COMMAND_TOPIC_PUMP_SIRAM = std::string(BASE_TOPIC) + "/pompa/siram/kontrol";
const std::string STATE_TOPIC_PUMP_SIRAM = std::string(BASE_TOPIC) + "/pompa/siram/status";

// =======================================================================
//                           COMMON DEFINITIONS
// =======================================================================
const int ULTRASONIC_MAX_DISTANCE_CM = 200;
const int DHT_TYPE = 22; // DHT22
const IPAddress PRIMARY_DNS(8, 8, 8, 8);     // Google's Public DNS

// =======================================================================
//                           SYSTEM CONFIGURATION
// =======================================================================
const int TANDON_MAX_HEIGHT_CM = 100;
const long SENSOR_PUBLISH_INTERVAL_MS = 5000;
const long HEARTBEAT_INTERVAL_MS = 10000;
const long WIFI_RECONNECT_DELAY_MS = 5000;
const long MQTT_RECONNECT_DELAY_MS = 5000;
const int MAX_RECONNECT_ATTEMPTS = 60;
const float PUMP_MS_PER_ML = (13.0 * 60.0 * 1000.0) / 200.0;
