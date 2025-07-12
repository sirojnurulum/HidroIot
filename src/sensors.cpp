#include "sensors.h"
#include "config.h" // For pin definitions and sensor configurations

// Include sensor libraries
#include <NewPing.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Sensor Objects
// These are now local to this file, not global in main.cpp
static NewPing sonar(ULTRASONIC_TRIGGER_PIN, ULTRASONIC_ECHO_PIN, ULTRASONIC_MAX_DISTANCE_CM);
static OneWire oneWire(ONE_WIRE_BUS);
static DallasTemperature ds18b20(&oneWire);
static DHT dht(DHT_PIN, DHT_TYPE);

void sensors_init() {
  Serial.println("Initializing sensors...");
  ds18b20.begin();
  dht.begin();
}

// Private helper function to read water temperature
static float read_water_temperature() {
  ds18b20.requestTemperatures();
  float tempC = ds18b20.getTempCByIndex(0);

  Serial.print("[Sensor] Water Temp: ");
  if (tempC == DEVICE_DISCONNECTED_C || tempC < -50 || tempC > 120) {
    Serial.println("Error: Sensor disconnected or invalid reading!");
    return NAN;
  }
  Serial.printf("%.2f °C\n", tempC);
  return tempC;
}

// Private helper function to read air temp/humidity
static void read_dht(float &temp, float &humidity) {
  humidity = dht.readHumidity();
  temp = dht.readTemperature();

  Serial.print("[Sensor] Air Temp/Humidity: ");
  if (isnan(humidity) || isnan(temp)) {
    Serial.println("Error: Failed to read from DHT sensor!");
    temp = NAN;
    humidity = NAN;
  } else {
    Serial.printf("Temp: %.2f °C, Humidity: %.2f %%\n", temp, humidity);
  }
}

// Private helper function to read ultrasonic sensor
static void read_ultrasonic(float &distance, float &level) {
  unsigned int usDistance = sonar.ping_cm();
  Serial.print("[Sensor] Ultrasonic - ");

  if (usDistance == 0 || usDistance >= ULTRASONIC_MAX_DISTANCE_CM) {
    Serial.println("Water Level: Out of Range / No Echo");
    distance = NAN;
    level = NAN;
  } else {
    distance = (float)usDistance;
    if (usDistance > TANDON_MAX_HEIGHT_CM) {
      level = 0;
      Serial.printf("Distance: %d cm (beyond max height), Level: %.1f cm (empty)\n", usDistance, level);
    } else {
      level = TANDON_MAX_HEIGHT_CM - usDistance;
      if (level > TANDON_MAX_HEIGHT_CM) {
        level = TANDON_MAX_HEIGHT_CM; // Clamp to full
      }
      Serial.printf("Distance: %d cm, Level: %.1f cm\n", usDistance, level);
    }
  }
}

void sensors_read_all(SensorValues &values) {
  Serial.println("\n--- START SENSOR READINGS ---");
  read_ultrasonic(values.waterDistanceCm, values.waterLevelCm);
  values.waterTempC = read_water_temperature();
  read_dht(values.airTempC, values.airHumidityPercent);
  Serial.println("--- END SENSOR READINGS ---\n");
}