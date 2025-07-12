#include "mqtt_handler.h"
#include "config.h"
#include <Arduino.h>
#include <WiFi.h> // For WiFiClient

#include "actuators.h" // To call actuator functions from MQTT callbacks

// --- Module-Private (Static) Variables ---
// Variabel ini sekarang bersifat lokal untuk file ini dan tersembunyi dari modul lain.
// These variables are now local to this file and hidden from other modules.
static WiFiClient espClient;
static PubSubClient mqttClient(espClient);
static unsigned long lastMqttReconnectAttempt = 0;

// --- Forward Declarations for Static Functions ---
// Deklarasi forward untuk fungsi-fungsi statis (privat) dalam modul ini.
// Forward declarations for static (private) functions within this module.
static void mqtt_callback(char *topic, byte *payload, unsigned int length);
static void mqtt_reconnect();
static bool publish_data(const char *topic, const char *payload, bool retain);

// --- Public Function Implementations ---

/**
 * @brief Menginisialisasi klien MQTT dengan server dan fungsi callback.
 * 
 * @brief Initializes the MQTT client with the server and callback function.
 */
void mqtt_init() {
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqtt_callback);
}

/**
 * @brief Loop untuk MQTT, menangani koneksi ulang non-blocking dan memproses pesan.
 * 
 * @brief Loop for MQTT, handles non-blocking reconnection and message processing.
 */
void mqtt_loop() {
  // Reconnect if connection is lost (and WiFi is available)
  // Sambungkan kembali jika koneksi terputus (dan WiFi tersedia)
  if (!mqttClient.connected() && WiFi.status() == WL_CONNECTED)
  {
    unsigned long now = millis();
    if (now - lastMqttReconnectAttempt > MQTT_RECONNECT_DELAY_MS)
    {
      lastMqttReconnectAttempt = now;
      mqtt_reconnect(); // Attempt to reconnect
    }
  }
  mqttClient.loop(); // Process messages and maintain connection
}

/**
 * @brief Memeriksa apakah klien MQTT sedang terhubung.
 * 
 * @brief Checks if the MQTT client is currently connected.
 */
bool mqtt_is_connected() {
  return mqttClient.connected();
}

/**
 * @brief Mempublikasikan semua data sensor yang valid ke topik MQTT masing-masing.
 * 
 * @brief Publishes all valid sensor data to their respective MQTT topics.
 */
void mqtt_publish_sensor_data(const SensorValues &values) {
  char payloadBuffer[10];

  // Publish Water Level & Distance
  // Publikasikan Level & Jarak Air
  if (isnan(values.waterLevelCm)) {
    publish_data(STATE_TOPIC_LEVEL, "unavailable", false);
    publish_data(STATE_TOPIC_DISTANCE, "unavailable", false);
  } else {
    dtostrf(values.waterDistanceCm, 4, 0, payloadBuffer);
    publish_data(STATE_TOPIC_DISTANCE, payloadBuffer, false);
    dtostrf(values.waterLevelCm, 4, 1, payloadBuffer);
    publish_data(STATE_TOPIC_LEVEL, payloadBuffer, false);
  }

  // Publish Water Temperature
  // Publikasikan Suhu Air
  if (isnan(values.waterTempC)) {
    publish_data(STATE_TOPIC_WATER_TEMPERATURE, "unavailable", false);
  } else {
    dtostrf(values.waterTempC, 5, 2, payloadBuffer);
    publish_data(STATE_TOPIC_WATER_TEMPERATURE, payloadBuffer, false);
  }

  // Publish Air Temperature & Humidity
  // Publikasikan Suhu & Kelembaban Udara
  if (isnan(values.airTempC)) {
    publish_data(STATE_TOPIC_AIR_TEMPERATURE, "unavailable", false);
    publish_data(STATE_TOPIC_HUMIDITY, "unavailable", false);
  } else {
    dtostrf(values.airTempC, 5, 2, payloadBuffer);
    publish_data(STATE_TOPIC_AIR_TEMPERATURE, payloadBuffer, false);
    dtostrf(values.airHumidityPercent, 5, 2, payloadBuffer);
    publish_data(STATE_TOPIC_HUMIDITY, payloadBuffer, false);
  }

  // Publish TDS
  // Publikasikan TDS
  if (isnan(values.tdsPpm)) {
    publish_data(STATE_TOPIC_TDS, "unavailable", false);
  } else {
    dtostrf(values.tdsPpm, 5, 2, payloadBuffer);
    publish_data(STATE_TOPIC_TDS, payloadBuffer, false);
  }
}

/**
 * @brief Mempublikasikan pesan heartbeat.
 * 
 * @brief Publishes a heartbeat message.
 */
void mqtt_publish_heartbeat() {
  publish_data(HEARTBEAT_TOPIC, "ESP32 Online", true);
}

/**
 * @brief Mempublikasikan pesan peringatan.
 * 
 * @brief Publishes an alert message.
 */
void mqtt_publish_alert(const char* alertMessage) {
  publish_data(MQTT_GLOBAL_ALERT_TOPIC, alertMessage, false);
}

/**
 * @brief Mempublikasikan pesan status generik (misalnya, status pompa).
 * 
 * @brief Publishes a generic state message (e.g., pump status).
 */
void mqtt_publish_state(const char* topic, const char* payload, bool retain) {
  publish_data(topic, payload, retain);
}


// --- Static (Private) Function Implementations ---

/**
 * @brief Fungsi pembantu privat untuk mempublikasikan data ke topik MQTT.
 *        Menangani pemeriksaan koneksi sebelum mempublikasikan.
 * 
 * @brief Private helper function to publish data to an MQTT topic.
 *        Handles connection checking before publishing.
 */
static bool publish_data(const char *topic, const char *payload, bool retain) {
  if (!mqttClient.connected()) {
    LOG_PRINT("     [PUB] MQTT Client not connected. Skipping publish to ");
    LOG_PRINTLN(topic);
    return false;
  }
  LOG_PRINT("     [PUB] Publishing to ");
  LOG_PRINT(topic);
  LOG_PRINT(": ");
  LOG_PRINTLN(payload);

  return mqttClient.publish(topic, payload, retain);
}

/**
 * @brief Fungsi callback yang dipanggil ketika pesan MQTT diterima.
 * 
 * @brief Callback function that is invoked when an MQTT message is received.
 */
static void mqtt_callback(char *topic, byte *payload, unsigned int length) {
  LOG_PRINT("\n[MQTT Command] Message arrived on topic: ");
  LOG_PRINT(topic);
  LOG_PRINT(". Payload: ");
  String messagePayload(reinterpret_cast<char*>(payload), length);
  LOG_PRINTLN(messagePayload);

  // Check if it's a system mode command
  if (strcmp(topic, COMMAND_TOPIC_SYSTEM_MODE) == 0) {
    actuators_handle_mode_command(messagePayload);
  } else {
    // Otherwise, assume it's a pump command and let the handler figure it out
    // Jika tidak, asumsikan itu adalah perintah pompa dan biarkan handler yang menanganinya
    actuators_handle_pump_command(topic, messagePayload);
  }
}

/**
 * @brief Mencoba untuk terhubung kembali ke broker MQTT.
 * 
 * @brief Attempts to reconnect to the MQTT broker.
 */
static void mqtt_reconnect() {
  LOG_PRINT("[MQTT] Attempting MQTT connection...");
  if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD,
                         AVAILABILITY_TOPIC, 0, true, "Offline")) {
    LOG_PRINTLN("connected");
    publish_data(AVAILABILITY_TOPIC, "Online", true);

    // Minta modul aktuator untuk mempublikasikan status awalnya
    actuators_publish_states(); // Ask the actuator module to publish its initial states

    mqttClient.subscribe(COMMAND_TOPIC_PUMP_A);
    mqttClient.subscribe(COMMAND_TOPIC_PUMP_B);
    mqttClient.subscribe(COMMAND_TOPIC_PUMP_PH);
    mqttClient.subscribe(COMMAND_TOPIC_SYSTEM_MODE);
    LOG_PRINTLN("[MQTT] Subscribed to pump control topics.");
  } else {
    LOG_PRINT("[MQTT] failed, rc=");
    LOG_PRINT(mqttClient.state());
    LOG_PRINTLN(" trying again in 5 seconds");
  }
}