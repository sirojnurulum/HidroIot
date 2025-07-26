#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <string>
#include <IPAddress.h>

// Pre-processor check to ensure an environment is selected
#if !defined(HYDROPONIC_INSTANCE)
  #error "HYDROPONIC_INSTANCE is not defined. Please select a build environment in platformio.ini (e.g., 'produksi' or 'penyemaian')."
#endif

// Include DHT library only if needed for the 'produksi' instance
#if HYDROPONIC_INSTANCE == 1
#include <DHT.h>
#endif

// =======================================================================
//                           LOGGING
// =======================================================================
// To enable, add '-D DEBUG_MODE' to build_flags in platformio.ini
#if defined(DEBUG_MODE)
  #define LOG_PRINT(...)    Serial.print(__VA_ARGS__)
  #define LOG_PRINTLN(...)  Serial.println(__VA_ARGS__)
  #define LOG_PRINTF(...)   Serial.printf(__VA_ARGS__)
#else
  #define LOG_PRINT(...)    (void)0
  #define LOG_PRINTLN(...)  (void)0
  #define LOG_PRINTF(...)   (void)0
#endif


// =======================================================================
//                           WIFI & MQTT CREDENTIALS
// =======================================================================
// Values are injected from platformio.ini
extern const char *WIFI_SSID;
extern const char *WIFI_PASSWORD;
extern const char *MQTT_SERVER;
extern const int MQTT_PORT;
extern const char *MQTT_USERNAME;
extern const char *MQTT_PASSWORD;
extern const char *MQTT_CLIENT_ID;
extern const char *BASE_TOPIC;


// =======================================================================
//                       COMMON SYSTEM CONFIGURATION
// =======================================================================
extern const IPAddress PRIMARY_DNS;
extern const int TANDON_MAX_HEIGHT_CM;
extern const long SENSOR_PUBLISH_INTERVAL_MS;
extern const long HEARTBEAT_INTERVAL_MS;
extern const long WIFI_RECONNECT_DELAY_MS;
extern const long MQTT_RECONNECT_DELAY_MS;
extern const int MAX_RECONNECT_ATTEMPTS;
extern const float PUMP_MS_PER_ML;


// =======================================================================
//                INSTANCE-SPECIFIC PINS & CONFIGURATION
// =======================================================================
#if HYDROPONIC_INSTANCE == 1 // --- 'Produksi' Instance ---
// PIN DEFINITIONS
extern const int ULTRASONIC_TRIGGER_PIN;
extern const int ULTRASONIC_ECHO_PIN;
extern const int ONE_WIRE_BUS;
extern const int DHT_PIN;
extern const int TDS_SENSOR_PIN;
extern const int BUZZER_PIN;
extern const int PUMP_NUTRISI_A_PIN;
extern const int PUMP_NUTRISI_B_PIN;
extern const int PUMP_PH_PIN;
extern const int PH_SENSOR_PIN;
extern const int PZEM_RX_PIN; // ESP32 RX2, connected to PZEM TX
extern const int PZEM_TX_PIN; // ESP32 TX2, connected to PZEM RX

// SYSTEM CONFIG
extern const int ULTRASONIC_MAX_DISTANCE_CM;
extern const int DHT_TYPE;
extern const float WATER_LEVEL_CRITICAL_CM;
extern const float TDS_LOWER_THRESHOLD_PPM;
extern const float DOSING_AMOUNT_ML;
extern const long AUTO_DOSING_CHECK_INTERVAL_MS;
extern const float TDS_K_VALUE;
extern const float TDS_TEMP_COEFF;
// pH Sensor Calibration Constants (MUST BE ADJUSTED BY USER)
extern const float PH_CALIBRATION_VOLTAGE_7; // Voltage reading in pH 7.0 buffer solution
extern const float PH_CALIBRATION_VOLTAGE_4; // Voltage reading in pH 4.0 buffer solution
extern const float PH_LOWER_THRESHOLD;
extern const float PH_UPPER_THRESHOLD;

#elif HYDROPONIC_INSTANCE == 2 // --- 'Penyemaian' Instance ---
// PIN DEFINITIONS
extern const int PUMP_SIRAM_PIN;

// SYSTEM CONFIG (No specific config for this instance yet)

#endif


// =======================================================================
//                           MQTT TOPICS
// =======================================================================
// These are constructed in config.cpp from the BASE_TOPIC
extern const std::string STATE_TOPIC_LEVEL;
extern const std::string STATE_TOPIC_DISTANCE;
extern const std::string STATE_TOPIC_WATER_TEMPERATURE;
extern const std::string STATE_TOPIC_AIR_TEMPERATURE;
extern const std::string STATE_TOPIC_HUMIDITY;
extern const std::string STATE_TOPIC_TDS;
extern const std::string STATE_TOPIC_PH;
extern const std::string STATE_TOPIC_VOLTAGE;
extern const std::string STATE_TOPIC_CURRENT;
extern const std::string STATE_TOPIC_POWER;
extern const std::string STATE_TOPIC_ENERGY;
extern const std::string STATE_TOPIC_FREQUENCY;
extern const std::string STATE_TOPIC_PF;
extern const std::string AVAILABILITY_TOPIC;
extern const std::string HEARTBEAT_TOPIC;
extern const std::string MQTT_GLOBAL_ALERT_TOPIC;

// Command & State Topics
extern const std::string COMMAND_TOPIC_SYSTEM_MODE;
extern const std::string STATE_TOPIC_SYSTEM_MODE;
extern const std::string COMMAND_TOPIC_PUMP_A;
extern const std::string STATE_TOPIC_PUMP_A;
extern const std::string COMMAND_TOPIC_PUMP_B;
extern const std::string STATE_TOPIC_PUMP_B;
extern const std::string COMMAND_TOPIC_PUMP_PH;
extern const std::string STATE_TOPIC_PUMP_PH;
extern const std::string COMMAND_TOPIC_PUMP_SIRAM;
extern const std::string STATE_TOPIC_PUMP_SIRAM;

#endif // CONFIG_H
