#ifndef CONFIG_H
#define CONFIG_H

// Include library for DHT_TYPE definition
#include <DHT.h>

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

// =======================================================================
//                           PIN DEFINITIONS
// =======================================================================
// Sensor Ultrasonik JSN-SR04T
// Ultrasonic Sensor JSN-SR04T
extern const int ULTRASONIC_TRIGGER_PIN;
extern const int ULTRASONIC_ECHO_PIN;
extern const int ULTRASONIC_MAX_DISTANCE_CM; // Jarak maksimal sensor dalam cm / Maximum sensor distance in cm

// Sensor Suhu DS18B20 (Suhu Air)
// Water Temperature Sensor DS18B20
extern const int ONE_WIRE_BUS; // Pin GPIO yang terhubung ke data pin DS18B20 / GPIO pin connected to DS18B20 data pin

// Sensor Suhu & Kelembaban DHT22 (Suhu & Kelembaban Udara)
// Air Temperature & Humidity Sensor DHT22
extern const int DHT_PIN;      // Pin GPIO yang terhubung ke pin DA/Data pada sensor DHT / GPIO pin connected to DHT sensor DA/Data pin
extern const int DHT_TYPE; // Menggunakan DHT22 (AM2302) / Using DHT22 (AM2302)

// TDS Sensor
// TDS Sensor
extern const int TDS_SENSOR_PIN; // Pin GPIO untuk sinyal analog TDS / GPIO pin for TDS analog signal

// Buzzer Peringatan
// Alert Buzzer
extern const int BUZZER_PIN; // Pin GPIO untuk Buzzer / GPIO pin for Buzzer

// Pompa Peristaltik (Relay/Modul MOSFET HIGH LEVEL TRIGGER: HIGH = ON, LOW = OFF)
// Peristaltic Pumps (Relay/MOSFET Module HIGH LEVEL TRIGGER: HIGH = ON, LOW = OFF)
extern const int PUMP_NUTRISI_A_PIN;
extern const int PUMP_NUTRISI_B_PIN;
extern const int PUMP_PH_PIN;

// =======================================================================
//                           SYSTEM CONFIGURATION
// =======================================================================
// Uncomment the line below to enable detailed serial logging for debugging
// Komentari baris di bawah ini untuk menonaktifkan logging serial untuk rilis
#define DEBUG_MODE

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

extern const int TANDON_MAX_HEIGHT_CM; // Tinggi tandon air Anda dalam cm / Your water reservoir height in cm

// Interval Waktu (dalam milidetik)
// Time Intervals (in milliseconds)
extern const long SENSOR_PUBLISH_INTERVAL_MS; // Kirim data sensor setiap 5 detik / Send sensor data every 5 seconds
extern const long HEARTBEAT_INTERVAL_MS;     // Kirim heartbeat setiap 10 detik / Send heartbeat every 10 seconds
extern const long WIFI_RECONNECT_DELAY_MS;    // Delay jika gagal konek WiFi / Delay on WiFi connection failure
extern const long MQTT_RECONNECT_DELAY_MS;    // Delay jika gagal konek MQTT / Delay on MQTT connection failure
extern const int MAX_RECONNECT_ATTEMPTS;        // Jumlah maksimum percobaan koneksi ulang / Maximum number of reconnection attempts

// Batas Ambang Peringatan (Sesuaikan sesuai kebutuhan Anda)
// Alert Threshold (Adjust to your needs)
extern const float WATER_LEVEL_CRITICAL_CM; // Jika air <= 20 cm, beri peringatan / If water <= 20 cm, trigger alert

// Kalibrasi Pompa
// Pump Calibration
extern const float PUMP_MS_PER_ML; // Waktu (ms) yang dibutuhkan pompa untuk memompa 1 ml cairan / Time (ms) required for the pump to dispense 1 ml of liquid

// Konfigurasi Dosis Otomatis (Auto-Dosing)
// Auto-Dosing Configuration
extern const float TDS_LOWER_THRESHOLD_PPM;      // Jika TDS < nilai ini, mulai dosis / If TDS < this value, start dosing
extern const float DOSING_AMOUNT_ML;             // Jumlah (ml) nutrisi yang ditambahkan per siklus dosis / Amount (ml) of nutrient to add per dosing cycle
extern const long AUTO_DOSING_CHECK_INTERVAL_MS; // Jeda minimum antar pengecekan dosis otomatis / Minimum interval between auto-dosing checks


// Kalibrasi Sensor TDS (sesuaikan berdasarkan larutan kalibrasi Anda)
// TDS Sensor Calibration (adjust based on your calibration solution)
extern const float TDS_K_VALUE; // Nilai K untuk konversi tegangan ke PPM / K value for voltage to PPM conversion
extern const float TDS_TEMP_COEFF; // Koefisien kompensasi suhu untuk TDS per °C / Temperature compensation coefficient for TDS per °C

// =======================================================================
//                           MQTT TOPICS
// =======================================================================
// Status Sensor
// Sensor Status
extern const char *STATE_TOPIC_LEVEL;               // Level air (cm) / Water level (cm)
extern const char *STATE_TOPIC_DISTANCE;     // Jarak sensor ke air (cm) / Sensor distance to water (cm)
extern const char *STATE_TOPIC_WATER_TEMPERATURE;     // Suhu air (°C) / Water temperature (°C)
extern const char *STATE_TOPIC_AIR_TEMPERATURE;     // Suhu udara (°C) / Air temperature (°C)
extern const char *STATE_TOPIC_HUMIDITY; // Kelembaban udara (%) / Air humidity (%)
extern const char *STATE_TOPIC_TDS;                   // TDS (ppm)

// Status Sistem
// System Status
extern const char *AVAILABILITY_TOPIC;        // LWT (Last Will and Testament) untuk status online/offline / LWT for online/offline status
extern const char *HEARTBEAT_TOPIC;    // Heartbeat untuk indikasi ESP32 masih hidup / Heartbeat to indicate ESP32 is alive
extern const char *MQTT_GLOBAL_ALERT_TOPIC; // Topik khusus untuk semua peringatan / Special topic for all alerts

// Mode Sistem
// System Mode
extern const char *COMMAND_TOPIC_SYSTEM_MODE; // Topik untuk mengatur mode sistem / Topic to set the system mode
extern const char *STATE_TOPIC_SYSTEM_MODE;   // Topik untuk melaporkan mode sistem saat ini / Topic to report the current system mode
// Kontrol & Status Pompa Peristaltik
// Peristaltic Pump Control & Status
extern const char *COMMAND_TOPIC_PUMP_A;
extern const char *STATE_TOPIC_PUMP_A;
extern const char *COMMAND_TOPIC_PUMP_B;
extern const char *STATE_TOPIC_PUMP_B;
extern const char *COMMAND_TOPIC_PUMP_PH;
extern const char *STATE_TOPIC_PUMP_PH;

#endif // CONFIG_H