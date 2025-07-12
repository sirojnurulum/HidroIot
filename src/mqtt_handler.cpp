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
  // Reconnect if connection is lost (and WiFi is available)
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

bool mqtt_is_connected() {
  return mqttClient.connected();
}

void mqtt_publish_sensor_data(const SensorValues &values) {
  char payloadBuffer[10];

  // Publish Water Level & Distance
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
  if (isnan(values.waterTempC)) {
    publish_data(STATE_TOPIC_WATER_TEMPERATURE, "unavailable", false);
  } else {
    dtostrf(values.waterTempC, 5, 2, payloadBuffer);
    publish_data(STATE_TOPIC_WATER_TEMPERATURE, payloadBuffer, false);
  }

  // Publish Air Temperature & Humidity
  if (isnan(values.airTempC)) {
    publish_data(STATE_TOPIC_AIR_TEMPERATURE, "unavailable", false);
    publish_data(STATE_TOPIC_HUMIDITY, "unavailable", false);
  } else {
    dtostrf(values.airTempC, 5, 2, payloadBuffer);
    publish_data(STATE_TOPIC_AIR_TEMPERATURE, payloadBuffer, false);
    dtostrf(values.airHumidityPercent, 5, 2, payloadBuffer);
    publish_data(STATE_TOPIC_HUMIDITY, payloadBuffer, false);
  }
}

void mqtt_publish_heartbeat() {
  publish_data(HEARTBEAT_TOPIC, "ESP32 Online", true);
}

void mqtt_publish_alert(const char* alertMessage) {
  publish_data(MQTT_GLOBAL_ALERT_TOPIC, alertMessage, false);
}

void mqtt_publish_state(const char* topic, const char* payload, bool retain) {
  publish_data(topic, payload, retain);
}


// --- Static (Private) Function Implementations ---

static bool publish_data(const char *topic, const char *payload, bool retain) {
  if (!mqttClient.connected()) {
    Serial.print("     [PUB] MQTT Client not connected. Skipping publish to ");
    Serial.println(topic);
    return false;
  }
  Serial.print("     [PUB] Publishing to ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(payload);

  return mqttClient.publish(topic, payload, retain);
}

static void mqtt_callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("\n[MQTT Command] Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Payload: ");
  String messagePayload(reinterpret_cast<char*>(payload), length);
  Serial.println(messagePayload);

  // Forward the command to the actuators module to handle it
  actuators_handle_pump_command(topic, messagePayload);
}

static void mqtt_reconnect() {
  Serial.print("[MQTT] Attempting MQTT connection...");
  if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD,
                         AVAILABILITY_TOPIC, 0, true, "Offline")) {
    Serial.println("connected");
    publish_data(AVAILABILITY_TOPIC, "Online", true);

    actuators_publish_states(); // Ask the actuator module to publish its initial states

    mqttClient.subscribe(COMMAND_TOPIC_PUMP_A);
    mqttClient.subscribe(COMMAND_TOPIC_PUMP_B);
    mqttClient.subscribe(COMMAND_TOPIC_PUMP_PH);
    Serial.println("[MQTT] Subscribed to pump control topics.");
  } else {
    Serial.print("[MQTT] failed, rc=");
    Serial.print(mqttClient.state());
    Serial.println(" trying again in 5 seconds");
  }
}