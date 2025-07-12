#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <PubSubClient.h>
#include "sensors.h" // For SensorValues struct

// Initializes the MQTT client and sets the callback function.
void mqtt_init();

// Handles MQTT connection and processes incoming messages. Call this in the main loop().
void mqtt_loop();

// Publishes all sensor data from the provided struct to their respective topics.
void mqtt_publish_sensor_data(const SensorValues &values);

// Publishes a heartbeat message to indicate the device is online.
void mqtt_publish_heartbeat();

// Publishes a generic alert message to the global alert topic.
void mqtt_publish_alert(const char* alertMessage);

// Publishes a generic state message to a topic. This will be used by the actuators module.
void mqtt_publish_state(const char* topic, const char* payload, bool retain);

// Returns true if the MQTT client is currently connected.
bool mqtt_is_connected();

#endif // MQTT_HANDLER_H
