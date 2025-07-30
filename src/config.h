/**
 * @file config.h
 * @brief Central configuration file for the hydroponics system.
 *
 * This file declares all global constants, pin definitions, and MQTT topics.
 */
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <string>
#include <IPAddress.h>

// --- Preprocessor Macros for Stringification ---
// These macros allow us to turn a build flag (like greenhouse_a) into a string literal ("greenhouse_a").
// This is a robust way to handle instance IDs.
#define XSTR(s) #s
#define STR(s) XSTR(s)

// Pre-processor check to ensure an instance ID is defined during compilation.
#ifndef HYDROPONIC_INSTANCE_ID
  #error "HYDROPONIC_INSTANCE_ID is not defined. Please select a build environment in platformio.ini (e.g., 'greenhouse_a')."
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
/// @brief Your WiFi network's SSID. Injected from credentials.ini.
extern const char *WIFI_SSID;
/// @brief Your WiFi network's password. Injected from credentials.ini.
extern const char *WIFI_PASSWORD;
/// @brief The address of your MQTT broker. Injected from credentials.ini.
extern const char *MQTT_SERVER;
/// @brief The port for your MQTT broker. Injected from credentials.ini.
extern const int MQTT_PORT;
/// @brief The username for your MQTT broker. Injected from credentials.ini.
extern const char *MQTT_USERNAME;
/// @brief The password for your MQTT broker. Injected from credentials.ini.
extern const char *MQTT_PASSWORD;
/// @brief The unique client ID for this ESP32 device.
extern const char *MQTT_CLIENT_ID;
/// @brief The base topic for all MQTT messages from this device.
extern const char *BASE_TOPIC;


// =======================================================================
//                       COMMON SYSTEM CONFIGURATION
// =======================================================================
/// @brief Primary DNS server to use for network lookups.
extern const IPAddress PRIMARY_DNS;
/// @brief The maximum height of the water reservoir in centimeters. Used to calculate water level from distance.
extern const int TANDON_MAX_HEIGHT_CM;
/// @brief The interval (in milliseconds) at which sensor data is read and published.
extern const long SENSOR_PUBLISH_INTERVAL_MS;
/// @brief The interval (in milliseconds) at which a heartbeat message is sent to MQTT.
extern const long HEARTBEAT_INTERVAL_MS;
/// @brief The delay (in milliseconds) before attempting to reconnect to WiFi.
extern const long WIFI_RECONNECT_DELAY_MS;
/// @brief The delay (in milliseconds) before attempting to reconnect to the MQTT broker.
extern const long MQTT_RECONNECT_DELAY_MS;
/// @brief The maximum number of times to attempt reconnection before pausing.
extern const int MAX_RECONNECT_ATTEMPTS;
/// @brief Pump calibration factor: milliseconds required to pump one milliliter of liquid.
extern const float PUMP_MS_PER_ML;


// =======================================================================
//                INSTANCE-SPECIFIC PINS & CONFIGURATION
// =======================================================================
/// @brief GPIO pin for the ultrasonic sensor's trigger.
extern const int ULTRASONIC_TRIGGER_PIN;
/// @brief GPIO pin for the ultrasonic sensor's echo.
extern const int ULTRASONIC_ECHO_PIN;
/// @brief GPIO pin for the One-Wire bus (DS18B20 water temperature sensor).
extern const int ONE_WIRE_BUS;
/// @brief GPIO pin for the DHT22 air temperature and humidity sensor.
extern const int DHT_PIN;
/// @brief Analog GPIO pin for the TDS sensor.
extern const int TDS_SENSOR_PIN;
/// @brief GPIO pin for the alert buzzer.
extern const int BUZZER_PIN;
/// @brief GPIO pin connected to the relay for Nutrient Pump A.
extern const int PUMP_NUTRISI_A_PIN;
/// @brief GPIO pin connected to the relay for Nutrient Pump B.
extern const int PUMP_NUTRISI_B_PIN;
/// @brief GPIO pin connected to the relay for the pH dosing pump.
extern const int PUMP_PH_PIN;
/// @brief GPIO pin connected to the relay for the watering pump.
extern const int PUMP_SIRAM_PIN;
/// @brief Analog GPIO pin for the pH sensor.
extern const int PH_SENSOR_PIN;
/// @brief GPIO pin for the PZEM-004T's TX, connected to the ESP32's RX2.
extern const int PZEM_RX_PIN; // ESP32 RX2, connected to PZEM TX
/// @brief GPIO pin for the PZEM-004T's RX, connected to the ESP32's TX2.
extern const int PZEM_TX_PIN; // ESP32 TX2, connected to PZEM RX

// SYSTEM CONFIG
/// @brief The maximum distance (in cm) the ultrasonic sensor should measure.
extern const int ULTRASONIC_MAX_DISTANCE_CM;
/// @brief The type of DHT sensor being used (e.g., DHT22).
extern const int DHT_TYPE;
/// @brief The water level (in cm) below which a critical alert is triggered.
extern const float WATER_LEVEL_CRITICAL_CM;
/// @brief The K-value for TDS sensor calibration. This may need adjustment.
extern const float TDS_K_VALUE;
/// @brief The temperature coefficient for TDS compensation.
extern const float TDS_TEMP_COEFF;
// pH Sensor Calibration Constants (MUST BE ADJUSTED BY USER)
/// @brief The voltage reading from the pH sensor when submerged in a pH 7.0 buffer solution.
extern const float PH_CALIBRATION_VOLTAGE_7; // Voltage reading in pH 7.0 buffer solution
/// @brief The voltage reading from the pH sensor when submerged in a pH 4.0 buffer solution.
extern const float PH_CALIBRATION_VOLTAGE_4; // Voltage reading in pH 4.0 buffer solution


// =======================================================================
//                           MQTT TOPICS
// =======================================================================
// These are constructed in config.cpp from the BASE_TOPIC
/// @brief MQTT topic for publishing the calculated water level.
extern const std::string STATE_TOPIC_LEVEL;
/// @brief MQTT topic for publishing the raw distance from the ultrasonic sensor.
extern const std::string STATE_TOPIC_DISTANCE;
/// @brief MQTT topic for publishing the water temperature.
extern const std::string STATE_TOPIC_WATER_TEMPERATURE;
/// @brief MQTT topic for publishing the air temperature.
extern const std::string STATE_TOPIC_AIR_TEMPERATURE;
/// @brief MQTT topic for publishing the air humidity.
extern const std::string STATE_TOPIC_HUMIDITY;
/// @brief MQTT topic for publishing the TDS (nutrient concentration).
extern const std::string STATE_TOPIC_TDS;
/// @brief MQTT topic for publishing the water's pH value.
extern const std::string STATE_TOPIC_PH;
/// @brief MQTT topic for publishing the electrical voltage.
extern const std::string STATE_TOPIC_VOLTAGE;
/// @brief MQTT topic for publishing the electrical current.
extern const std::string STATE_TOPIC_CURRENT;
/// @brief MQTT topic for publishing the electrical power.
extern const std::string STATE_TOPIC_POWER;
/// @brief MQTT topic for publishing the total energy consumption.
extern const std::string STATE_TOPIC_ENERGY;
/// @brief MQTT topic for publishing the electrical frequency.
extern const std::string STATE_TOPIC_FREQUENCY;
/// @brief MQTT topic for publishing the power factor.
extern const std::string STATE_TOPIC_PF;
/// @brief MQTT topic for publishing the device's online/offline status (LWT).
extern const std::string AVAILABILITY_TOPIC;
/// @brief MQTT topic for publishing periodic heartbeat messages.
extern const std::string HEARTBEAT_TOPIC;
/// @brief MQTT topic for publishing system-wide alerts.
extern const std::string MQTT_GLOBAL_ALERT_TOPIC;

// Command & State Topics
/// @brief MQTT topic for receiving system mode commands (e.g., NUTRITION, CLEANER).
extern const std::string COMMAND_TOPIC_SYSTEM_MODE;
/// @brief MQTT topic for publishing the current system mode.
extern const std::string STATE_TOPIC_SYSTEM_MODE;
/// @brief MQTT topic for receiving commands for Nutrient Pump A.
extern const std::string COMMAND_TOPIC_PUMP_A;
/// @brief MQTT topic for publishing the state of Nutrient Pump A.
extern const std::string STATE_TOPIC_PUMP_A;
/// @brief MQTT topic for receiving commands for Nutrient Pump B.
extern const std::string COMMAND_TOPIC_PUMP_B;
/// @brief MQTT topic for publishing the state of Nutrient Pump B.
extern const std::string STATE_TOPIC_PUMP_B;
/// @brief MQTT topic for receiving commands for the pH dosing pump.
extern const std::string COMMAND_TOPIC_PUMP_PH;
/// @brief MQTT topic for publishing the state of the pH dosing pump.
extern const std::string STATE_TOPIC_PUMP_PH;
/// @brief MQTT topic for receiving commands for the watering pump.
extern const std::string COMMAND_TOPIC_PUMP_SIRAM;
/// @brief MQTT topic for publishing the state of the watering pump.
extern const std::string STATE_TOPIC_PUMP_SIRAM;

#endif // CONFIG_H
