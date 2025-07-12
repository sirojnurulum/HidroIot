#ifndef CONFIG_H
#define CONFIG_H

// Include library for DHT_TYPE definition
#include <DHT.h>

// =======================================================================
//                           WIFI CONFIGURATION
// =======================================================================
extern const char *WIFI_SSID;                      // SSID WiFi Anda
extern const char *WIFI_PASSWORD; // Password WiFi Anda

// =======================================================================
//                           MQTT CONFIGURATION
// =======================================================================
extern const char *MQTT_SERVER; // Domain MQTT Broker Anda
extern const int MQTT_PORT;
extern const char *MQTT_USERNAME;
extern const char *MQTT_PASSWORD;
extern const char *MQTT_CLIENT_ID; // ID unik untuk perangkat ini di MQTT

// =======================================================================
//                           PIN DEFINITIONS
// =======================================================================
// Sensor Ultrasonik JSN-SR04T
extern const int ULTRASONIC_TRIGGER_PIN;
extern const int ULTRASONIC_ECHO_PIN;
extern const int ULTRASONIC_MAX_DISTANCE_CM; // Jarak maksimal sensor dalam cm

// Sensor Suhu DS18B20 (Suhu Air)
extern const int ONE_WIRE_BUS; // Pin GPIO yang terhubung ke data pin DS18B20

// Sensor Suhu & Kelembaban DHT22 (Suhu & Kelembaban Udara)
extern const int DHT_PIN;      // Pin GPIO yang terhubung ke pin DA/Data pada sensor DHT
extern const int DHT_TYPE; // Menggunakan DHT22 (AM2302)

// Buzzer Peringatan
extern const int BUZZER_PIN; // Pin GPIO untuk Buzzer

// Pompa Peristaltik (Relay/Modul MOSFET HIGH LEVEL TRIGGER: HIGH = ON, LOW = OFF)
extern const int PUMP_NUTRISI_A_PIN;
extern const int PUMP_NUTRISI_B_PIN;
extern const int PUMP_PH_PIN;

// =======================================================================
//                           SYSTEM CONFIGURATION
// =======================================================================
extern const int TANDON_MAX_HEIGHT_CM; // Tinggi tandon air Anda dalam cm

// Interval Waktu (dalam milidetik)
extern const long SENSOR_PUBLISH_INTERVAL_MS; // Kirim data sensor setiap 5 detik
extern const long HEARTBEAT_INTERVAL_MS;     // Kirim heartbeat setiap 10 detik
extern const long WIFI_RECONNECT_DELAY_MS;    // Delay jika gagal konek WiFi
extern const long MQTT_RECONNECT_DELAY_MS;    // Delay jika gagal konek MQTT
extern const int MAX_RECONNECT_ATTEMPTS;        // Jumlah maksimum percobaan koneksi ulang

// Batas Ambang Peringatan (Sesuaikan sesuai kebutuhan Anda)
extern const float WATER_LEVEL_CRITICAL_CM; // Jika air <= 20 cm, beri peringatan

// Kalibrasi Pompa
extern const float PUMP_MS_PER_ML; // Waktu (ms) yang dibutuhkan pompa untuk memompa 1 ml cairan

// =======================================================================
//                           MQTT TOPICS
// =======================================================================
// Status Sensor
extern const char *STATE_TOPIC_LEVEL;               // Level air (cm)
extern const char *STATE_TOPIC_DISTANCE;     // Jarak sensor ke air (cm)
extern const char *STATE_TOPIC_WATER_TEMPERATURE;     // Suhu air (°C)
extern const char *STATE_TOPIC_AIR_TEMPERATURE;     // Suhu udara (°C)
extern const char *STATE_TOPIC_HUMIDITY; // Kelembaban udara (%)

// Status Sistem
extern const char *AVAILABILITY_TOPIC;        // LWT (Last Will and Testament) untuk status online/offline
extern const char *HEARTBEAT_TOPIC;    // Heartbeat untuk indikasi ESP32 masih hidup
extern const char *MQTT_GLOBAL_ALERT_TOPIC; // Topik khusus untuk semua peringatan

// Kontrol & Status Pompa Peristaltik
extern const char *COMMAND_TOPIC_PUMP_A;
extern const char *STATE_TOPIC_PUMP_A;
extern const char *COMMAND_TOPIC_PUMP_B;
extern const char *STATE_TOPIC_PUMP_B;
extern const char *COMMAND_TOPIC_PUMP_PH;
extern const char *STATE_TOPIC_PUMP_PH;

#endif // CONFIG_H