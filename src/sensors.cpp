#include "sensors.h"

#if HYDROPONIC_INSTANCE == 1
#include "config.h" // For pin definitions and sensor configurations

// Include sensor libraries
#include <NewPing.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Sensor Objects
static NewPing sonar(ULTRASONIC_TRIGGER_PIN, ULTRASONIC_ECHO_PIN, ULTRASONIC_MAX_DISTANCE_CM);
static OneWire oneWire(ONE_WIRE_BUS);
static DallasTemperature ds18b20(&oneWire);
static DHT dht(DHT_PIN, DHT_TYPE);

void sensors_init() {
  LOG_PRINTLN("Initializing sensors...");
  ds18b20.begin();
  dht.begin();
  analogReadResolution(12);
}

static float read_water_temperature() {
  ds18b20.requestTemperatures();
  float tempC = ds18b20.getTempCByIndex(0);

  LOG_PRINT("[Sensor] Water Temp: ");
  if (tempC == DEVICE_DISCONNECTED_C || tempC < -50 || tempC > 120) {
    LOG_PRINTLN("Error: Sensor disconnected or invalid reading!");
    return NAN;
  }
  LOG_PRINTF("%.2f °C\n", tempC);
  return tempC;
}

static void read_dht(float &temp, float &humidity) {
  humidity = dht.readHumidity();
  temp = dht.readTemperature();

  LOG_PRINT("[Sensor] Air Temp/Humidity: ");
  if (isnan(humidity) || isnan(temp)) {
    LOG_PRINTLN("Error: Failed to read from DHT sensor!");
    temp = NAN;
    humidity = NAN;
  } else {
    LOG_PRINTF("Temp: %.2f °C, Humidity: %.2f %%\n", temp, humidity);
  }
}

static void read_ultrasonic(float &distance, float &level) {
  unsigned int usDistance = sonar.ping_cm();
  LOG_PRINT("[Sensor] Ultrasonic - ");

  if (usDistance == 0 || usDistance >= ULTRASONIC_MAX_DISTANCE_CM) {
    LOG_PRINTLN("Water Level: Out of Range / No Echo");
    distance = NAN;
    level = NAN;
  } else {
    distance = (float)usDistance;
    if (usDistance > TANDON_MAX_HEIGHT_CM) {
      level = 0;
      LOG_PRINTF("Distance: %d cm (beyond max height), Level: %.1f cm (empty)\n", usDistance, level);
    } else {
      level = TANDON_MAX_HEIGHT_CM - usDistance;
      if (level > TANDON_MAX_HEIGHT_CM) {
        level = TANDON_MAX_HEIGHT_CM;
      }
      LOG_PRINTF("Distance: %d cm, Level: %.1f cm\n", usDistance, level);
    }
  }
}

static float read_tds(float waterTemp) {
  const float VREF = 3.3;
  const int ADC_RESOLUTION = 4095;
  int adcValue = analogRead(TDS_SENSOR_PIN);
  float voltage = adcValue * VREF / ADC_RESOLUTION;

  LOG_PRINT("[Sensor] TDS - ");

  if (voltage > 2.4) {
    LOG_PRINTLN("Error: Invalid voltage reading!");
    return NAN;
  }

  float rawTds = voltage * TDS_K_VALUE;
  float compensatedTds = rawTds / (1.0 + TDS_TEMP_COEFF * (waterTemp - 25.0));

  LOG_PRINTF("Voltage: %.2fV, Compensated TDS: %.2f ppm\n", voltage, compensatedTds);
  return compensatedTds;
}

void sensors_read_all(SensorValues &values) {
  LOG_PRINTLN("\n--- START SENSOR READINGS ---");
  read_ultrasonic(values.waterDistanceCm, values.waterLevelCm);
  values.waterTempC = read_water_temperature();
  values.tdsPpm = isnan(values.waterTempC) ? NAN : read_tds(values.waterTempC);
  read_dht(values.airTempC, values.airHumidityPercent);
  LOG_PRINTLN("--- END SENSOR READINGS ---\n");
}

#endif // HYDROPONIC_INSTANCE == 1
