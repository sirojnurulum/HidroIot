#include "mqtt_handler.h"
#include "config.h"
#include <Arduino.h>
#include <WiFi.h> // For WiFiClient

#include "actuators.h" // To call actuator functions from MQTT callbacks

// --- Module-Private (Static) Variables ---
static WiFiClient espClient;
static PubSubClient mqttClient(espClient);
static unsigned long lastMqttReconnectAttempt = 0;

// --- Forward Declarations for Static Functions ---
static void mqtt_callback(char *topic, byte *payload, unsigned int length);
static void mqtt_reconnect();
static bool publish_data(const char *topic, const char *payload, bool retain);

// --- Public Function Implementations ---

void mqtt_init() {
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqtt_callback);
}

void mqtt_loop() {
  if (!mqttClient.connected() && WiFi.status() == WL_CONNECTED)
  {
    unsigned long now = millis();
    if (now - lastMqttReconnectAttempt > MQTT_RECONNECT_DELAY_MS)
    {
      lastMqttReconnectAttempt = now;
      mqtt_reconnect();
    }
  }
  mqttClient.loop();
}

bool mqtt_is_connected() {
  return mqttClient.connected();
}

#if HYDROPONIC_INSTANCE == 1
void mqtt_publish_sensor_data(const SensorValues &values) {
  char payloadBuffer[10];

  if (isnan(values.waterLevelCm)) {
    publish_data(STATE_TOPIC_LEVEL.c_str(), "unavailable", false);
    publish_data(STATE_TOPIC_DISTANCE.c_str(), "unavailable", false);
  } else {
    dtostrf(values.waterDistanceCm, 4, 0, payloadBuffer);
    publish_data(STATE_TOPIC_DISTANCE.c_str(), payloadBuffer, false);
    dtostrf(values.waterLevelCm, 4, 1, payloadBuffer);
    publish_data(STATE_TOPIC_LEVEL.c_str(), payloadBuffer, false);
  }

  if (isnan(values.waterTempC)) {
    publish_data(STATE_TOPIC_WATER_TEMPERATURE.c_str(), "unavailable", false);
  } else {
    dtostrf(values.waterTempC, 5, 2, payloadBuffer);
    publish_data(STATE_TOPIC_WATER_TEMPERATURE.c_str(), payloadBuffer, false);
  }

  if (isnan(values.airTempC)) {
    publish_data(STATE_TOPIC_AIR_TEMPERATURE.c_str(), "unavailable", false);
    publish_data(STATE_TOPIC_HUMIDITY.c_str(), "unavailable", false);
  } else {
    dtostrf(values.airTempC, 5, 2, payloadBuffer);
    publish_data(STATE_TOPIC_AIR_TEMPERATURE.c_str(), payloadBuffer, false);
    dtostrf(values.airHumidityPercent, 5, 2, payloadBuffer);
    publish_data(STATE_TOPIC_HUMIDITY.c_str(), payloadBuffer, false);
  }

  if (isnan(values.tdsPpm)) {
    publish_data(STATE_TOPIC_TDS.c_str(), "unavailable", false);
  } else {
    dtostrf(values.tdsPpm, 5, 2, payloadBuffer);
    publish_data(STATE_TOPIC_TDS.c_str(), payloadBuffer, false);
  }
}
#endif

void mqtt_publish_heartbeat() {
  publish_data(HEARTBEAT_TOPIC.c_str(), "ESP32 Online", true);
}

#if HYDROPONIC_INSTANCE == 1
void mqtt_publish_alert(const char* alertMessage) {
  publish_data(MQTT_GLOBAL_ALERT_TOPIC.c_str(), alertMessage, false);
}
#endif

void mqtt_publish_state(const char* topic, const char* payload, bool retain) {
  publish_data(topic, payload, retain);
}


// --- Static (Private) Function Implementations ---

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

static void mqtt_callback(char *topic, byte *payload, unsigned int length) {
  LOG_PRINT("\n[MQTT Command] Message arrived on topic: ");
  LOG_PRINT(topic);
  LOG_PRINT(". Payload: ");
  String messagePayload(reinterpret_cast<char*>(payload), length);
  LOG_PRINTLN(messagePayload);

#if HYDROPONIC_INSTANCE == 1
  if (topic == COMMAND_TOPIC_SYSTEM_MODE) {
    actuators_handle_mode_command(messagePayload);
  } else {
    actuators_handle_pump_command(topic, messagePayload);
  }
#elif HYDROPONIC_INSTANCE == 2
  actuators_handle_pump_command(topic, messagePayload);
#endif
}

static void mqtt_reconnect() {
  LOG_PRINT("[MQTT] Attempting MQTT connection...");
  if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD,
                         AVAILABILITY_TOPIC.c_str(), 0, true, "Offline")) {
    LOG_PRINTLN("connected");
    publish_data(AVAILABILITY_TOPIC.c_str(), "Online", true);

    actuators_publish_states();

#if HYDROPONIC_INSTANCE == 1
    mqttClient.subscribe(COMMAND_TOPIC_PUMP_A.c_str());
    mqttClient.subscribe(COMMAND_TOPIC_PUMP_B.c_str());
    mqttClient.subscribe(COMMAND_TOPIC_PUMP_PH.c_str());
    mqttClient.subscribe(COMMAND_TOPIC_SYSTEM_MODE.c_str());
#elif HYDROPONIC_INSTANCE == 2
    mqttClient.subscribe(COMMAND_TOPIC_PUMP_SIRAM.c_str());
#endif
    LOG_PRINTLN("[MQTT] Subscribed to relevant control topics.");
  } else {
    LOG_PRINT("[MQTT] failed, rc=");
    LOG_PRINT(mqttClient.state());
    LOG_PRINTLN(" trying again in 5 seconds");
  }
}
