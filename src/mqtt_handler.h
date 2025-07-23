#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <string>

#if HYDROPONIC_INSTANCE == 1
#include "sensors.h" // For SensorValues struct
#endif

/**
 * @brief Initializes the MQTT client and sets the server and callback.
 */
void mqtt_init();

/**
 * @brief Main loop for the MQTT handler. Manages connection and processes messages.
 *        Should be called repeatedly from the main application loop.
 */
void mqtt_loop();

/**
 * @brief Checks if the MQTT client is currently connected to the broker.
 * @return true if connected, false otherwise.
 */
bool mqtt_is_connected();

/**
 * @brief Publishes a generic state message to a specific topic.
 * @param topic The destination MQTT topic.
 * @param payload The message payload to send.
 * @param retain True to make the message a retained message, false otherwise.
 */
void mqtt_publish_state(const std::string& topic, const char* payload, bool retain);

/**
 * @brief Publishes a heartbeat message to the designated heartbeat topic.
 */
void mqtt_publish_heartbeat();

#if HYDROPONIC_INSTANCE == 1
/**
 * @brief Publishes all sensor data from the provided SensorValues struct.
 * @param values The struct containing the latest sensor data.
 */
void mqtt_publish_sensor_data(const SensorValues &values);

/**
 * @brief Publishes an alert message to the designated global alert topic.
 * @param alertMessage The content of the alert message.
 */
void mqtt_publish_alert(const char* alertMessage);
#endif

#endif // MQTT_HANDLER_H
