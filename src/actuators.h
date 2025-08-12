/**
 * @file actuators.h
 * @brief Public interface for the actuator module.
 *
 * This file declares all functions related to controlling physical actuators
 * like pumps and the buzzer, and for handling system state logic.
 */
#ifndef ACTUATORS_H
#define ACTUATORS_H

#include "sensors.h" // For SensorValues struct

// --- Automation State Structure ---
/**
 * @brief Structure to hold the status of all automation features.
 */
struct AutomationState {
    bool auto_dosing_enabled;     ///< Whether auto pH & TDS dosing is enabled
    bool auto_refill_enabled;     ///< Whether auto-refill tandon is enabled
    bool auto_irrigation_enabled; ///< Whether auto irrigation is enabled
};

// --- Global Automation State ---
/// @brief Global instance of automation state, accessible throughout the system
extern AutomationState automation_state;

// (Optional) Future: expose a getter for current system mode if needed by other modules
// enum SystemMode { NUTRITION, CLEANER };
// SystemMode get_current_mode();

/**
 * @brief Initializes all actuator pins.
 * Sets the GPIO pins for all pumps and the buzzer to OUTPUT mode and ensures
 * they are in a default OFF state. This should be called once in `setup()`.
 */
void actuators_init();

/**
 * @brief Main loop for the actuator module.
 * This function must be called repeatedly in the main `loop()`.
 * It checks for timed pump completions and enforces safety rules, like
 * preventing the reservoir from overflowing.
 * @param currentValues A reference to the latest sensor readings.
 */
void actuators_loop(const SensorValues& currentValues);

/**
 * @brief Handles incoming MQTT commands for all pumps.
 * Parses the topic and payload to determine which pump to control and how.
 * Supports volume-based control (ml) for dosing pumps and duration-based
 * control (s) for the watering pump. Also handles the "OFF" command.
 * @param topic The MQTT topic the command was received on.
 * @param command The payload of the MQTT command.
 */
void actuators_handle_pump_command(const char* topic, const char* command);

/**
 * @brief Handles incoming MQTT commands for changing the system mode (e.g., NUTRITION, CLEANER).
 * @param command The payload of the mode command.
 */
void actuators_handle_mode_command(const char* command);

/**
 * @brief Handles incoming MQTT commands for automation enable/disable.
 * @param topic The MQTT topic the command was received on.
 * @param command The payload of the automation command ("ON" or "OFF").
 */
void actuators_handle_automation_command(const char* topic, const char* command);

/**
 * @brief Publishes the current state of all automation settings to their respective MQTT topics.
 * This is useful on startup or after an MQTT reconnection to ensure Home Assistant
 * has the correct automation state information.
 */
void actuators_publish_automation_states();

/**
 * @brief Updates the alert status (buzzer and MQTT alert) based on sensor values.
 * @param values The SensorValues struct containing the latest sensor data.
 */
void actuators_update_alert_status(const SensorValues& values);

/**
 * @brief Publishes the current state of all actuators to their respective MQTT topics.
 * This is useful on startup or after an MQTT reconnection to ensure Home Assistant
 * has the correct state information.
 */
void actuators_publish_states();

#endif // ACTUATORS_H
