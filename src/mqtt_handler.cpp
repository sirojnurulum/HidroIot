#include "mqtt_handler.h"
#include "config.h"
#include "actuators.h" // To call actuator functions from MQTT callbacks
#include <WiFi.h>
#include <PubSubClient.h>

// --- Module-Private (Static) Variables ---
static WiFiClient espClient;
static PubSubClient mqttClient(espClient);
static unsigned long lastMqttReconnectAttempt = 0;

// --- Forward Declarations for Static Functions ---
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

#if HYDROPONIC_INSTANCE == 1
void mqtt_publish_sensor_data(const SensorValues &values) {
    char payloadBuffer[16]; // Increased buffer size for safety

    // Helper lambda to publish a float value or "unavailable" if it's NAN
    auto publish_float = [&](const std::string& topic, float value, const char* format) {
        if (isnan(value)) {
            mqtt_publish_state(topic, "unavailable", false);
        } else {
            sprintf(payloadBuffer, format, value);
            mqtt_publish_state(topic, payloadBuffer, false);
        }
    };

    publish_float(STATE_TOPIC_LEVEL, values.waterLevelCm, "%.1f");
    publish_float(STATE_TOPIC_DISTANCE, values.waterDistanceCm, "%.0f");
    publish_float(STATE_TOPIC_WATER_TEMPERATURE, values.waterTempC, "%.2f");
    publish_float(STATE_TOPIC_AIR_TEMPERATURE, values.airTempC, "%.2f");
    publish_float(STATE_TOPIC_HUMIDITY, values.airHumidityPercent, "%.2f");
    publish_float(STATE_TOPIC_TDS, values.tdsPpm, "%.1f");

    // Publish PZEM-004T data
    publish_float(STATE_TOPIC_VOLTAGE, values.pzemVoltage, "%.1f");
    publish_float(STATE_TOPIC_CURRENT, values.pzemCurrent, "%.3f");
    publish_float(STATE_TOPIC_POWER, values.pzemPower, "%.1f");
    publish_float(STATE_TOPIC_ENERGY, values.pzemEnergy, "%.3f");
    publish_float(STATE_TOPIC_FREQUENCY, values.pzemFrequency, "%.1f");
    publish_float(STATE_TOPIC_PF, values.pzemPowerFactor, "%.2f");

    // Publish pH data
    publish_float(STATE_TOPIC_PH, values.phValue, "%.2f");
}

void mqtt_publish_alert(const char* alertMessage) {
    mqtt_publish_state(MQTT_GLOBAL_ALERT_TOPIC, alertMessage, false);
}
#endif

// --- Static (Private) Function Implementations ---

static void mqtt_reconnect() {
    if (WiFi.status() != WL_CONNECTED) {
        LOG_PRINTLN("[MQTT] Cannot reconnect, WiFi is not connected.");
        return;
    }

    LOG_PRINTF("[MQTT] Attempting to connect to broker at %s:%d...\n", MQTT_SERVER, MQTT_PORT);
    
    // Attempt to connect with Last Will and Testament (LWT)
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD,
                           AVAILABILITY_TOPIC.c_str(), 1, true, "Offline")) {
        LOG_PRINTLN("[MQTT] Connection successful!");
        
        // Publish "Online" to the LWT topic to show we are connected
        mqtt_publish_state(AVAILABILITY_TOPIC, "Online", true);
        
        // Subscribe to command topics
        subscribe_to_topics();

        // Publish the current state of all actuators
        actuators_publish_states();

    } else {
        LOG_PRINTF("[MQTT] Connection failed, rc=%d. Will try again later.\n", mqttClient.state());
    }
}

static void subscribe_to_topics() {
    LOG_PRINTLN("[MQTT] Subscribing to command topics...");
#if HYDROPONIC_INSTANCE == 1
    mqttClient.subscribe(COMMAND_TOPIC_PUMP_A.c_str());
    mqttClient.subscribe(COMMAND_TOPIC_PUMP_B.c_str());
    mqttClient.subscribe(COMMAND_TOPIC_PUMP_PH.c_str());
    mqttClient.subscribe(COMMAND_TOPIC_SYSTEM_MODE.c_str());
#elif HYDROPONIC_INSTANCE == 2
    mqttClient.subscribe(COMMAND_TOPIC_PUMP_SIRAM.c_str());
#endif
}

static void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    // Convert payload to a null-terminated string for safe handling
    char messageBuffer[length + 1];
    memcpy(messageBuffer, payload, length);
    messageBuffer[length] = '\0';

    LOG_PRINTF("\n[MQTT] Command received on topic: %s\n", topic);
    LOG_PRINTF("  > Payload: %s\n", messageBuffer);

    // Route the command to the correct handler in the actuators module
#if HYDROPONIC_INSTANCE == 1
    if (strcmp(topic, COMMAND_TOPIC_SYSTEM_MODE.c_str()) == 0) {
        actuators_handle_mode_command(messageBuffer);
    } else {
        // Assume any other topic is a pump command
        actuators_handle_pump_command(topic, messageBuffer);
    }
#elif HYDROPONIC_INSTANCE == 2
    // For the seeding instance, all commands are pump commands
    if (strcmp(topic, COMMAND_TOPIC_PUMP_SIRAM.c_str()) == 0) {
        actuators_handle_pump_command(topic, messageBuffer);
    }
#endif
}
