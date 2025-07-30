/**
 * @file mqtt_handler.cpp
 * @brief Implements the logic for MQTT communication.
 *
 * This module handles connecting to the MQTT broker, subscribing to command topics,
 * processing incoming messages, and publishing sensor data and device states.
 */

#include "mqtt_handler.h"
#include "config.h"
#include "actuators.h" // To call actuator functions from MQTT callbacks
#include <WiFi.h>
#include <PubSubClient.h>

// --- Module-Private (Static) Variables ---

/// @brief The underlying WiFi client for the MQTT connection.
static WiFiClient espClient;
/// @brief The main PubSubClient object for handling MQTT communication.
static PubSubClient mqttClient(espClient);
/// @brief Tracks the last time a reconnection attempt was made to prevent spamming.
static unsigned long lastMqttReconnectAttempt = 0;

// --- Forward Declarations for Static (Private) Functions ---
static void mqtt_callback(char* topic, byte* payload, unsigned int length);
static void mqtt_reconnect();
static void subscribe_to_topics();

// --- Public Function Implementations ---

void mqtt_init() {
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setCallback(mqtt_callback);
}

void mqtt_loop() {
    // If not connected, and it's time for a new attempt, try to reconnect.
    if (!mqttClient.connected()) {
        unsigned long now = millis();
        if (now - lastMqttReconnectAttempt >= MQTT_RECONNECT_DELAY_MS) {
            lastMqttReconnectAttempt = now;
            mqtt_reconnect();
        }
    }
    // This is the core of the PubSubClient library, must be called regularly.
    mqttClient.loop();
}

bool mqtt_is_connected() {
    return mqttClient.connected();
}

void mqtt_publish_state(const std::string& topic, const char* payload, bool retain) {
    if (!mqttClient.connected()) {
        LOG_PRINTF("[MQTT] WARN: Cannot publish to %s, client not connected.\n", topic.c_str());
        return;
    }
    LOG_PRINTF("  [MQTT] Publishing to %s: %s\n", topic.c_str(), payload);
    mqttClient.publish(topic.c_str(), payload, retain);
}

void mqtt_publish_heartbeat() {
    // The payload can be anything, "Online" is descriptive.
    mqtt_publish_state(HEARTBEAT_TOPIC, "Online", true);
}

void mqtt_publish_sensor_data(const SensorValues &values) {
    char payloadBuffer[16]; // A safe buffer size for float-to-string conversion.

    // Helper lambda to publish a float value or "unavailable" if it's NAN.
    // This avoids code duplication and makes the logic cleaner.
    auto publish_float = [&](const std::string& topic, float value, const char* format) {
        if (isnan(value)) {
            mqtt_publish_state(topic, "unavailable", false);
        } else {
            // Use snprintf for buffer safety. It won't write past the buffer size.
            snprintf(payloadBuffer, sizeof(payloadBuffer), format, value);
            mqtt_publish_state(topic, payloadBuffer, false);
        }
    };

    publish_float(STATE_TOPIC_LEVEL, values.waterLevelCm, "%.1f");
    publish_float(STATE_TOPIC_DISTANCE, values.waterDistanceCm, "%.0f");
    publish_float(STATE_TOPIC_WATER_TEMPERATURE, values.waterTempC, "%.2f");
    publish_float(STATE_TOPIC_AIR_TEMPERATURE, values.airTempC, "%.2f");
    publish_float(STATE_TOPIC_HUMIDITY, values.airHumidityPercent, "%.2f");
    publish_float(STATE_TOPIC_TDS, values.tdsPpm, "%.1f");
    publish_float(STATE_TOPIC_PH, values.phValue, "%.2f");

    // Publish PZEM-004T data
    publish_float(STATE_TOPIC_VOLTAGE, values.pzemVoltage, "%.1f");
    publish_float(STATE_TOPIC_CURRENT, values.pzemCurrent, "%.3f");
    publish_float(STATE_TOPIC_POWER, values.pzemPower, "%.1f");
    publish_float(STATE_TOPIC_ENERGY, values.pzemEnergy, "%.3f");
    publish_float(STATE_TOPIC_FREQUENCY, values.pzemFrequency, "%.1f");
    publish_float(STATE_TOPIC_PF, values.pzemPowerFactor, "%.2f");
}

void mqtt_publish_alert(const char* alertMessage) {
    mqtt_publish_state(MQTT_GLOBAL_ALERT_TOPIC, alertMessage, false);
}


// --- Static (Private) Function Implementations ---

/**
 * @brief Attempts to reconnect to the MQTT broker.
 * This function handles the connection logic, including setting the
 * Last Will and Testament (LWT) message.
 */
static void mqtt_reconnect() {
    if (WiFi.status() != WL_CONNECTED) {
        LOG_PRINTLN("[MQTT] Cannot reconnect, WiFi is not connected.");
        return;
    }

    LOG_PRINTF("[MQTT] Attempting to connect to broker at %s:%d...\n", MQTT_SERVER, MQTT_PORT);
    
    // Attempt to connect with Last Will and Testament (LWT).
    // If the device disconnects ungracefully, the broker will automatically
    // publish "Offline" to the availability topic.
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD,
                           AVAILABILITY_TOPIC.c_str(), 1, true, "Offline")) {
        LOG_PRINTLN("[MQTT] Connection successful!");
        
        // Publish "Online" to the LWT topic to show we are connected.
        mqtt_publish_state(AVAILABILITY_TOPIC, "Online", true);
        
        // Subscribe to all necessary command topics.
        subscribe_to_topics();

        // Publish the current state of all actuators to sync with Home Assistant.
        actuators_publish_states();

    } else {
        LOG_PRINTF("[MQTT] Connection failed, rc=%d. Will try again later.\n", mqttClient.state());
    }
}

/**
 * @brief Subscribes to all command topics after a successful connection.
 */
static void subscribe_to_topics() {
    LOG_PRINTLN("[MQTT] Subscribing to all command topics...");
    mqttClient.subscribe(COMMAND_TOPIC_PUMP_A.c_str());
    mqttClient.subscribe(COMMAND_TOPIC_PUMP_B.c_str());
    mqttClient.subscribe(COMMAND_TOPIC_PUMP_PH.c_str());
    mqttClient.subscribe(COMMAND_TOPIC_PUMP_SIRAM.c_str());
    mqttClient.subscribe(COMMAND_TOPIC_SYSTEM_MODE.c_str());
}

/**
 * @brief The callback function that is executed when a message is received.
 * It routes the incoming command to the appropriate handler in the actuators module.
 * @param topic The topic the message was received on.
 * @param payload The message payload.
 * @param length The length of the payload.
 */
static void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    // Convert payload to a null-terminated string for safe handling.
    char messageBuffer[length + 1];
    memcpy(messageBuffer, payload, length);
    messageBuffer[length] = '\0';

    LOG_PRINTF("\n[MQTT] Command received on topic: %s\n", topic);
    LOG_PRINTF("  > Payload: %s\n", messageBuffer);

    std::string topic_str(topic);

    // Route the command to the correct handler in the actuators module.
    if (topic_str == COMMAND_TOPIC_SYSTEM_MODE) {
        actuators_handle_mode_command(messageBuffer);
    } else {
        // Assume any other subscribed topic is a pump command.
        // The actuator module will find the correct pump based on the topic.
        actuators_handle_pump_command(topic, messageBuffer);
    }
}
