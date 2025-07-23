#ifndef SENSORS_H
#define SENSORS_H

#if HYDROPONIC_INSTANCE == 1

#include <Arduino.h> // For isnan()

/**
 * @struct SensorValues
 * @brief A single structure to hold all sensor readings, making it easy to
 *        pass data between modules.
 */
struct SensorValues {
    float waterLevelCm;
    float waterDistanceCm;
    float waterTempC;
    float airTempC;
    float airHumidityPercent;
    float tdsPpm;
    // Fields for PZEM-004T Power Sensor
    float pzemVoltage;
    float pzemCurrent;
    float pzemPower;
    float pzemEnergy;
    float pzemFrequency;
    float pzemPowerFactor;
    float phValue;
};

/**
 * @brief Initializes all connected sensors. Call this once in setup().
 */
void sensors_init();

/**
 * @brief Reads all sensors and populates the provided SensorValues struct.
 * @param values Reference to the SensorValues struct to be filled with data.
 */
void sensors_read_all(SensorValues &values);

#endif // HYDROPONIC_INSTANCE == 1

#endif // SENSORS_H
