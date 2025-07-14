#include "config.h"

// =======================================================================
//                           WIFI CONFIGURATION
// =======================================================================
const char *WIFI_SSID = "Baceprot";
const char *WIFI_PASSWORD = "***";

// =======================================================================
//                           MQTT CONFIGURATION
// =======================================================================
const char *MQTT_SERVER = "ha.o-m-b.id";
const int MQTT_PORT = 1883;
const char *MQTT_USERNAME = "Baceprot";
const char *MQTT_PASSWORD = "***";
const char *MQTT_CLIENT_ID = "esp32hidro"; // Harus unik untuk setiap perangkat di broker / Must be unique for each device on the broker

// =======================================================================
//                           PIN DEFINITIONS
// =======================================================================
const int ULTRASONIC_TRIGGER_PIN = 5;
const int ULTRASONIC_ECHO_PIN = 4;
const int ULTRASONIC_MAX_DISTANCE_CM = 200;
const int ONE_WIRE_BUS = 16;
const int DHT_PIN = 13;
const int DHT_TYPE = DHT22; // Menggunakan DHT22 (AM2302) / Using DHT22 (AM2302)
const int TDS_SENSOR_PIN = 34;
const int BUZZER_PIN = 17;
const int PUMP_NUTRISI_A_PIN = 25;
const int PUMP_NUTRISI_B_PIN = 26;
const int PUMP_PH_PIN = 27;

// =======================================================================
//                           SYSTEM CONFIGURATION
// =======================================================================
const int TANDON_MAX_HEIGHT_CM = 100;         // Tinggi tandon air Anda dalam cm / Your water reservoir height in cm
const long SENSOR_PUBLISH_INTERVAL_MS = 5000; // Kirim data sensor setiap 5 detik / Send sensor data every 5 seconds
const long HEARTBEAT_INTERVAL_MS = 10000;     // Kirim heartbeat setiap 10 detik / Send heartbeat every 10 seconds
const long WIFI_RECONNECT_DELAY_MS = 5000;    // Delay jika gagal konek WiFi / Delay on WiFi connection failure
const long MQTT_RECONNECT_DELAY_MS = 5000;    // Delay jika gagal konek MQTT / Delay on MQTT connection failure
const int MAX_RECONNECT_ATTEMPTS = 60;        // Jumlah maksimum percobaan koneksi ulang / Maximum number of reconnection attempts
const float WATER_LEVEL_CRITICAL_CM = 20.0;   // Jika air <= 20 cm, beri peringatan / If water <= 20 cm, trigger alert

// Konfigurasi Dosis Otomatis
// Auto-Dosing Configuration
const float TDS_LOWER_THRESHOLD_PPM = 650.0;      // Target TDS adalah 700-800, jadi dosis jika di bawah 650 / Target TDS is 700-800, so dose if below 650
const float DOSING_AMOUNT_ML = 20.0;              // Tambahkan 20ml setiap kali dosis / Add 20ml each time
const long AUTO_DOSING_CHECK_INTERVAL_MS = 1800000; // Cek setiap 30 menit (30 * 60 * 1000) / Check every 30 minutes

// Kalibrasi Sensor TDS - SESUAIKAN NILAI INI
// TDS Sensor Calibration - ADJUST THIS VALUE
// Nilai default ini adalah titik awal. Kalibrasi untuk akurasi.
const float TDS_K_VALUE = 635.40; // Calibrated with 1382ppm solution @ ~2.175V.
const float TDS_TEMP_COEFF = 0.02; // Standar 2% per derajat Celsius / Standard 2% per degree Celsius


// Kalibrasi Pompa (berdasarkan 200ml dalam 13 menit)
// Pump Calibration (based on 200ml in 13 minutes)
const float PUMP_MS_PER_ML = (13.0 * 60.0 * 1000.0) / 200.0; // Hasilnya: 3900.0 ms/ml / Result: 3900.0 ms/ml


// =======================================================================
//                           MQTT TOPICS
// =======================================================================
const char *STATE_TOPIC_LEVEL = "hidroponik/air/level_cm";
const char *STATE_TOPIC_DISTANCE = "hidroponik/air/jarak_sensor_cm";
const char *STATE_TOPIC_WATER_TEMPERATURE = "hidroponik/air/suhu_c";
const char *STATE_TOPIC_AIR_TEMPERATURE = "hidroponik/udara/suhu_c";
const char *STATE_TOPIC_HUMIDITY = "hidroponik/udara/kelembaban_persen";
const char *STATE_TOPIC_TDS = "hidroponik/air/tds_ppm";
const char *AVAILABILITY_TOPIC = "tele/esp32hidro/LWT";
const char *HEARTBEAT_TOPIC = "tele/esp32hidro/HEARTBEAT";
const char *MQTT_GLOBAL_ALERT_TOPIC = "hidroponik/peringatan";
const char *COMMAND_TOPIC_SYSTEM_MODE = "hidroponik/sistem/mode/kontrol";
const char *STATE_TOPIC_SYSTEM_MODE = "hidroponik/sistem/mode/status";
const char *COMMAND_TOPIC_PUMP_A = "hidroponik/pompa/nutrisi_a/kontrol";
const char *STATE_TOPIC_PUMP_A = "hidroponik/pompa/nutrisi_a/status";
const char *COMMAND_TOPIC_PUMP_B = "hidroponik/pompa/nutrisi_b/kontrol";
const char *STATE_TOPIC_PUMP_B = "hidroponik/pompa/nutrisi_b/status";
const char *COMMAND_TOPIC_PUMP_PH = "hidroponik/pompa/ph/kontrol";
const char *STATE_TOPIC_PUMP_PH = "hidroponik/pompa/ph/status";