/**
 * @file mqtt_handler.h
 * @brief Public interface for the MQTT communication module.
 *
 * This file declares all functions for initializing the MQTT client, managing
 * the connection, and publishing various types of data to the MQTT broker.
 */
#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <string>
#include "sensors.h" // For SensorValues struct

/**
 * @brief Initializes the MQTT client.
 * Sets the MQTT server address, port, and the callback function for handling
 * incoming messages. This should be called once in `setup()`.
 */
void mqtt_init();

/**
 * @brief Main loop for the MQTT handler.
 * This function must be called repeatedly in the main `loop()`. It maintains
 * the connection to the broker and processes incoming messages.
 */
void mqtt_loop();

/**
 * @brief Checks if the MQTT client is currently connected to the broker.
 * @return true if connected, false otherwise.
 */
bool mqtt_is_connected();

/**
 * @brief Publishes a generic state message to a specific topic.
 * @param topic The destination MQTT topic as a std::string.
 * @param payload The message payload to send as a C-style string.
 * @param retain True to make the message a retained message, false otherwise.
 */
void mqtt_publish_state(const std::string& topic, const char* payload, bool retain);

/**
 * @brief Publishes a heartbeat message to the designated heartbeat topic.
 * This signals to the system that the device is alive and running.
 */
void mqtt_publish_heartbeat();

/**
 * @brief Publishes all sensor data from the provided SensorValues struct.
 * Iterates through the struct and publishes each value to its corresponding
 * MQTT topic. Handles NAN values by publishing "unavailable".
 * @param values The struct containing the latest sensor data.
 */
void mqtt_publish_sensor_data(const SensorValues &values);

/**
 * @brief Publishes an alert message to the designated global alert topic.
 * @param alertMessage The content of the alert message.
 */
void mqtt_publish_alert(const char* alertMessage);

#endif // MQTT_HANDLER_H
