/**
 * @file sensors.h
 * @brief Public interface for the sensor module.
 *
 * This file defines the data structure for holding sensor values and declares the functions for initializing and reading all sensors.
 */
#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h> // For isnan()

/**
 * @struct SensorValues
 * @brief A single structure to hold all sensor readings, making it easy to
 *        pass data between modules.
 */
struct SensorValues {
    /// @brief The calculated water level in centimeters from the top of the water to the bottom of the reservoir.
    float waterLevelCm;
    /// @brief The raw distance in centimeters from the ultrasonic sensor to the water surface.
    float waterDistanceCm;
    /// @brief The water temperature in degrees Celsius.
    float waterTempC;
    /// @brief The ambient air temperature in degrees Celsius.
    float airTempC;
    /// @brief The ambient air humidity in percent.
    float airHumidityPercent;
    /// @brief The Total Dissolved Solids (nutrient concentration) in parts per million (ppm).
    float tdsPpm;
    // Fields for PZEM-004T Power Sensor
    /// @brief The electrical voltage in Volts (V).
    float pzemVoltage;
    /// @brief The electrical current in Amperes (A).
    float pzemCurrent;
    /// @brief The active power in Watts (W).
    float pzemPower;
    /// @brief The total energy consumption in kilowatt-hours (kWh).
    float pzemEnergy;
    /// @brief The electrical frequency in Hertz (Hz).
    float pzemFrequency;
    /// @brief The power factor (ratio, unitless).
    float pzemPowerFactor;
    /// @brief The measured pH value of the water.
    float phValue;
};

/**
 * @brief Initializes all connected sensors.
 * This function should be called once in the `setup()` function. It prepares
 * the sensor libraries and hardware for reading.
 */
void sensors_init();

/**
 * @brief Reads all sensors and populates the provided SensorValues struct.
 * This function orchestrates the reading of each individual sensor and stores
 * the results in the `values` struct. It handles dependencies, such as
 * needing water temperature to calculate compensated TDS.
 * @param values Reference to the SensorValues struct to be filled with data.
 */
void sensors_read_all(SensorValues &values);

#endif // SENSORS_H
