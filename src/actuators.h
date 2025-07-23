#ifndef ACTUATORS_H
#define ACTUATORS_H

#if HYDROPONIC_INSTANCE == 1
#include "sensors.h" // For SensorValues struct
#endif

/**
 * @brief Initializes all actuator pins (pumps, buzzer).
 */
void actuators_init();

/**
 * @brief Handles the timed operation of actuators. Call this in the main loop().
 *        For example, to stop a pump after a specified duration.
 */
void actuators_loop();

/**
 * @brief Processes an incoming MQTT command to control a pump.
 *        Supports volume commands (e.g., "50") and "OFF".
 * @param topic The topic the command was received on.
 * @param command The payload of the command as a C-style string.
 */
void actuators_handle_pump_command(const char* topic, const char* command);

#if HYDROPONIC_INSTANCE == 1
/**
 * @brief Processes an incoming MQTT command to change the system mode.
 *        Supports "NUTRITION" and "CLEANER" modes.
 * @param command The payload of the command as a C-style string.
 */
void actuators_handle_mode_command(const char* command);

/**
 * @brief Updates the alert status (buzzer and MQTT alert) based on sensor values.
 * @param values The SensorValues struct containing the latest sensor data.
 */
void actuators_update_alert_status(const SensorValues& values);

/**
 * @brief Checks sensor values and performs automatic nutrient dosing if required.
 * @param values The SensorValues struct containing the latest sensor data.
 */
void actuators_auto_dose_nutrients(const SensorValues& values);
#endif

/**
 * @brief Publishes the current state of all pumps to MQTT.
 *        Typically called when an MQTT connection is successfully established.
 */
void actuators_publish_states();

#endif // ACTUATORS_H
