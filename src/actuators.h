#ifndef ACTUATORS_H
#define ACTUATORS_H

#include "sensors.h" // For SensorValues struct
#include <Arduino.h> // For String

// Initializes all actuator pins (pumps, buzzer).
void actuators_init();

// Handles the timed operation of actuators. Call this in the main loop().
void actuators_loop();

// Processes an incoming MQTT command to control a pump.
void actuators_handle_pump_command(const char* topic, const String& command);

// Updates the alert status (buzzer and MQTT alert) based on sensor values.
// This function will contain the logic that is currently in updateBuzzerAlert().
void actuators_update_alert_status(const SensorValues& values);

// Publishes the current state of all pumps to MQTT.
void actuators_publish_states();

#endif // ACTUATORS_H