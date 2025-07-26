#include "config.h"

// =======================================================================
//                           WIFI & MQTT CREDENTIALS
// =======================================================================
// These values are injected by platformio.ini via build_flags
const char *WIFI_SSID = ENV_WIFI_SSID;
const char *WIFI_PASSWORD = ENV_WIFI_PASSWORD;
const char *MQTT_USERNAME = ENV_MQTT_USER;
const char *MQTT_PASSWORD = ENV_MQTT_PASS;
const char *MQTT_SERVER = ENV_MQTT_SERVER;
const int MQTT_PORT = ENV_MQTT_PORT;


// =======================================================================
//                           INSTANCE-SPECIFIC DEFINITIONS
// =======================================================================
#if HYDROPONIC_INSTANCE == 1 // --- 'Produksi' Instance ---
const char *MQTT_CLIENT_ID = "esp32-hydroponic-prod";
const char *BASE_TOPIC = "hidroponik/produksi";

// PIN DEFINITIONS
const int ULTRASONIC_TRIGGER_PIN = 5;
const int ULTRASONIC_ECHO_PIN = 4;
const int ONE_WIRE_BUS = 18; // Moved from 16 to free up Serial2 RX
const int DHT_PIN = 13;
const int TDS_SENSOR_PIN = 34;
const int BUZZER_PIN = 19; // Moved from 17 to free up Serial2 TX
const int PUMP_NUTRISI_A_PIN = 25;
const int PUMP_NUTRISI_B_PIN = 26;
const int PUMP_PH_PIN = 27;
const int PH_SENSOR_PIN = 32;
const int PZEM_RX_PIN = 16; // ESP32 RX2, terhubung ke TX PZEM
const int PZEM_TX_PIN = 17; // ESP32 TX2, terhubung ke RX PZEM

// SYSTEM CONFIG
const int ULTRASONIC_MAX_DISTANCE_CM = 400;
const int DHT_TYPE = DHT22;
const float WATER_LEVEL_CRITICAL_CM = 20.0;
const float TDS_LOWER_THRESHOLD_PPM = 650.0;
const float DOSING_AMOUNT_ML = 20.0;
const long AUTO_DOSING_CHECK_INTERVAL_MS = 1800000; // 30 minutes
const float TDS_K_VALUE = 635.40;
const float TDS_TEMP_COEFF = 0.02;
// pH Sensor Calibration - IMPORTANT: These are placeholder values.
// You MUST calibrate your sensor and replace these values.
const float PH_CALIBRATION_VOLTAGE_7 = 1.65; // Example: 1.65V at pH 7.0
const float PH_CALIBRATION_VOLTAGE_4 = 2.15; // Example: 2.15V at pH 4.0
const float PH_LOWER_THRESHOLD = 5.8;
const float PH_UPPER_THRESHOLD = 6.8;

#elif HYDROPONIC_INSTANCE == 2 // --- 'Penyemaian' Instance ---
const char *MQTT_CLIENT_ID = "esp32-hydroponic-seed";
const char *BASE_TOPIC = "hidroponik/penyemaian";

// PIN DEFINITIONS
const int PUMP_SIRAM_PIN = 32;

#endif


// =======================================================================
//                           COMMON SYSTEM CONFIGURATION
// =======================================================================
const IPAddress PRIMARY_DNS(8, 8, 8, 8);
const int TANDON_MAX_HEIGHT_CM = 100;
const long SENSOR_PUBLISH_INTERVAL_MS = 5000;
const long HEARTBEAT_INTERVAL_MS = 10000;
const long WIFI_RECONNECT_DELAY_MS = 5000;
const long MQTT_RECONNECT_DELAY_MS = 5000;
const int MAX_RECONNECT_ATTEMPTS = 60;
// Pump calibration: (seconds per revolution * 60 * 1000) / ml per revolution
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
const std::string COMMAND_TOPIC_PUMP_SIRAM = std::string(BASE_TOPIC) + "/pompa/siram/kontrol";
const std::string STATE_TOPIC_PUMP_SIRAM = std::string(BASE_TOPIC) + "/pompa/siram/status";
