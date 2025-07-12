#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// A struct to hold all sensor readings in one place.
// This makes it easy to pass sensor data between modules.
struct SensorValues {
  float waterLevelCm;
  float waterDistanceCm;
  float waterTempC;
  float airTempC;
  float airHumidityPercent;
};

// Initializes all sensors. Call this in setup().
void sensors_init();

// Reads all sensors and populates the provided struct with the latest values.
// Invalid readings will be represented by NAN (Not a Number).
void sensors_read_all(SensorValues &values);

#endif // SENSORS_H
