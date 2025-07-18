#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <Arduino.h> // For IPAddress

// Include library for DHT_TYPE definition only if needed
#if HYDROPONIC_INSTANCE == 1
#include <DHT.h>
#endif

// =======================================================================
//                           WIFI CONFIGURATION
// =======================================================================
extern const char *WIFI_SSID;     // SSID WiFi Anda / Your WiFi SSID
extern const char *WIFI_PASSWORD; // Password WiFi Anda / Your WiFi Password

// =======================================================================
//                           MQTT CONFIGURATION
// =======================================================================
extern const char *MQTT_SERVER; // Domain MQTT Broker Anda / Your MQTT Broker domain
extern const int MQTT_PORT;
extern const char *MQTT_USERNAME;
extern const char *MQTT_PASSWORD;
extern const char *MQTT_CLIENT_ID; // ID unik untuk perangkat ini di MQTT / Unique ID for this device on MQTT

// Base topic untuk membedakan antar instance hidroponik
extern const char *BASE_TOPIC;

// =======================================================================
//                           PIN DEFINITIONS
// =======================================================================
#if HYDROPONIC_INSTANCE == 1
// Sensor Ultrasonik JSN-SR04T
extern const int ULTRASONIC_TRIGGER_PIN;
extern const int ULTRASONIC_ECHO_PIN;

// Sensor Suhu DS18B20 (Suhu Air)
extern const int ONE_WIRE_BUS;

// Sensor Suhu & Kelembaban DHT22 (Suhu & Kelembaban Udara)
extern const int DHT_PIN;

// TDS Sensor
extern const int TDS_SENSOR_PIN;

// Buzzer Peringatan
extern const int BUZZER_PIN;

// Pompa Peristaltik (Nutrisi & pH)
extern const int PUMP_NUTRISI_A_PIN;
extern const int PUMP_NUTRISI_B_PIN;
extern const int PUMP_PH_PIN;
#endif

// Pompa Penyiraman (hanya untuk penyemaian)
extern const int PUMP_SIRAM_PIN;

// =======================================================================
//                           SYSTEM CONFIGURATION
// =======================================================================
// Uncomment the line below to enable detailed serial logging for debugging
// Komentari baris di bawah ini untuk menonaktifkan logging serial untuk rilis
#define DEBUG_MODE

// Kode Status WiFi untuk Diagnostik (Referensi)
// WiFi Status Codes for Diagnostics (Reference)
// 0: WL_IDLE_STATUS      - Status idle
// 3: WL_CONNECTED        - Berhasil terhubung!
// 6: WL_DISCONNECTED     - Terputus setelah mencoba terhubung (seringkali karena masalah DHCP)

// Logging macros to easily enable/disable serial output
// Makro logging untuk mengaktifkan/menonaktifkan output serial dengan mudah
#ifdef DEBUG_MODE
  #define LOG_PRINT(...) Serial.print(__VA_ARGS__)
  #define LOG_PRINTLN(...) Serial.println(__VA_ARGS__)
  #define LOG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
  #define LOG_PRINT(...)
  #define LOG_PRINTLN(...)
  #define LOG_PRINTF(...)
#endif

extern const IPAddress PRIMARY_DNS;
extern const int TANDON_MAX_HEIGHT_CM;
extern const int ULTRASONIC_MAX_DISTANCE_CM;
extern const int DHT_TYPE;

// Interval Waktu (dalam milidetik)
extern const long SENSOR_PUBLISH_INTERVAL_MS;
extern const long HEARTBEAT_INTERVAL_MS;
extern const long WIFI_RECONNECT_DELAY_MS;
extern const long MQTT_RECONNECT_DELAY_MS;
extern const int MAX_RECONNECT_ATTEMPTS;

// Kalibrasi Pompa
extern const float PUMP_MS_PER_ML;

#if HYDROPONIC_INSTANCE == 1
// Batas Ambang Peringatan
extern const float WATER_LEVEL_CRITICAL_CM;

// Konfigurasi Dosis Otomatis (Auto-Dosing)
extern const float TDS_LOWER_THRESHOLD_PPM;
extern const float DOSING_AMOUNT_ML;
extern const long AUTO_DOSING_CHECK_INTERVAL_MS;

// Kalibrasi Sensor TDS
extern const float TDS_K_VALUE;
extern const float TDS_TEMP_COEFF;
#endif

// =======================================================================
//                           MQTT TOPICS
// =======================================================================
extern const std::string STATE_TOPIC_LEVEL;
extern const std::string STATE_TOPIC_DISTANCE;
extern const std::string STATE_TOPIC_WATER_TEMPERATURE;
extern const std::string STATE_TOPIC_AIR_TEMPERATURE;
extern const std::string STATE_TOPIC_HUMIDITY;
extern const std::string STATE_TOPIC_TDS;
extern const std::string AVAILABILITY_TOPIC;
extern const std::string HEARTBEAT_TOPIC;
extern const std::string MQTT_GLOBAL_ALERT_TOPIC;
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
