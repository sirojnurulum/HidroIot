#include "config.h"

// =======================================================================
//                           WIFI CONFIGURATION
// =======================================================================
const char *WIFI_SSID = "Baceprot";
const char *WIFI_PASSWORD = "ya.gak.tau.kok.tanya.saya";

// =======================================================================
//                           MQTT CONFIGURATION
// =======================================================================
const char *MQTT_SERVER = "ha.o-m-b.id";
const int MQTT_PORT = 1883;
const char *MQTT_USERNAME = "Baceprot";
const char *MQTT_PASSWORD = "Baceprot@IotHidro";
const char *MQTT_CLIENT_ID = "esp32hidro";

// =======================================================================
//                           PIN DEFINITIONS
// =======================================================================
const int ULTRASONIC_TRIGGER_PIN = 5;
const int ULTRASONIC_ECHO_PIN = 4;
const int ULTRASONIC_MAX_DISTANCE_CM = 200;
const int ONE_WIRE_BUS = 16;
const int DHT_PIN = 13;
const int DHT_TYPE = DHT22;
const int BUZZER_PIN = 17;
const int PUMP_NUTRISI_A_PIN = 25;
const int PUMP_NUTRISI_B_PIN = 26;
const int PUMP_PH_PIN = 27;

// =======================================================================
//                           SYSTEM CONFIGURATION
// =======================================================================
const int TANDON_MAX_HEIGHT_CM = 100;
const long SENSOR_PUBLISH_INTERVAL_MS = 5000;
const long HEARTBEAT_INTERVAL_MS = 10000;
const long WIFI_RECONNECT_DELAY_MS = 5000;
const long MQTT_RECONNECT_DELAY_MS = 5000;
const int MAX_RECONNECT_ATTEMPTS = 60;
const float WATER_LEVEL_CRITICAL_CM = 20.0;

// Kalibrasi Pompa (berdasarkan 200ml dalam 13 menit)
const float PUMP_MS_PER_ML = (13.0 * 60.0 * 1000.0) / 200.0; // Hasilnya: 3900.0

// =======================================================================
//                           MQTT TOPICS
// =======================================================================
const char *STATE_TOPIC_LEVEL = "hidroponik/air/level_cm";
const char *STATE_TOPIC_DISTANCE = "hidroponik/air/jarak_sensor_cm";
const char *STATE_TOPIC_WATER_TEMPERATURE = "hidroponik/air/suhu_c";
const char *STATE_TOPIC_AIR_TEMPERATURE = "hidroponik/udara/suhu_c";
const char *STATE_TOPIC_HUMIDITY = "hidroponik/udara/kelembaban_persen";
const char *AVAILABILITY_TOPIC = "tele/esp32hidro/LWT";
const char *HEARTBEAT_TOPIC = "tele/esp32hidro/HEARTBEART";
const char *MQTT_GLOBAL_ALERT_TOPIC = "hidroponik/peringatan";
const char *COMMAND_TOPIC_PUMP_A = "hidroponik/pompa/nutrisi_a/kontrol";
const char *STATE_TOPIC_PUMP_A = "hidroponik/pompa/nutrisi_a/status";
const char *COMMAND_TOPIC_PUMP_B = "hidroponik/pompa/nutrisi_b/kontrol";
const char *STATE_TOPIC_PUMP_B = "hidroponik/pompa/nutrisi_b/status";
const char *COMMAND_TOPIC_PUMP_PH = "hidroponik/pompa/ph/kontrol";
const char *STATE_TOPIC_PUMP_PH = "hidroponik/pompa/ph/status";