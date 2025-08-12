/**
 * @file sensors.cpp
 * @brief Implements the functionality for initializing and reading all hardware sensors.
 */

#include "sensors.h"
#include "config.h" // For pin definitions and sensor configurations

// Include all necessary sensor libraries
#include <NewPing.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <PZEM004Tv30.h>

// --- Module-Private (Static) Objects & Constants ---

/// @brief Sonar object for the JSN-SR04T ultrasonic sensor.
static NewPing sonar(ULTRASONIC_TRIGGER_PIN, ULTRASONIC_ECHO_PIN, ULTRASONIC_MAX_DISTANCE_CM);
/// @brief OneWire object for the DS18B20 temperature sensor's communication bus.
static OneWire oneWire(ONE_WIRE_BUS);
/// @brief DallasTemperature object to interface with the DS18B20 sensor.
static DallasTemperature ds18b20(&oneWire);
/// @brief DHT object for the DHT22 air temperature and humidity sensor.
static DHT dht(DHT_PIN, DHT_TYPE);
/// @brief PZEM004Tv30 object for the power meter, using hardware serial port 2.
static PZEM004Tv30 pzem(Serial2, PZEM_RX_PIN, PZEM_TX_PIN);

// --- Voltage validation constants for analog sensors ---
static const float TDS_MAX_VOLTAGE = 2.4f;
static const float TDS_MIN_VOLTAGE = 0.1f;
static const float PH_MAX_VOLTAGE = 3.2f;
static const float PH_MIN_VOLTAGE = 0.1f;


// --- Forward Declarations for Static (Private) Functions ---
static float read_water_temperature();
static void read_dht(float &temp, float &humidity);
static void read_ultrasonic(float &distance, float &level);
static float read_tds(float waterTemp);
static void read_pzem(SensorValues &values);
static float read_ph();


// --- Public Function Implementations ---

void sensors_init() {
  LOG_PRINTLN("[Sensors] Initializing...");
  ds18b20.begin();
  dht.begin();
  
  // Set the ADC resolution to 12-bit for higher precision on analog sensors.
  analogReadResolution(12);
  
  // Configure ADC for more accurate pH readings (same as calibration setup)
  analogSetWidth(12);              // Set resolusi ADC ke 12-bit (4096)
  analogSetAttenuation(ADC_11db);  // Set attenuation untuk range 0-3.3V
}

void sensors_read_all(SensorValues &values) {
  LOG_PRINTLN("\n--- Reading All Sensors ---");
  read_ultrasonic(values.waterDistanceCm, values.waterLevelCm);
  values.waterTempC = read_water_temperature();
  // Only read TDS if water temperature is valid, as it's needed for compensation.
  values.tdsPpm = isnan(values.waterTempC) ? NAN : read_tds(values.waterTempC);
  read_dht(values.airTempC, values.airHumidityPercent);
  read_pzem(values);
  values.phValue = read_ph();
  LOG_PRINTLN("--- Finished Sensor Readings ---\n");
}


// --- Static (Private) Function Implementations ---

/**
 * @brief Reads the water temperature from the DS18B20 sensor.
 * @return The temperature in degrees Celsius, or NAN on failure.
 */
static float read_water_temperature() {
  ds18b20.requestTemperatures();
  float tempC = ds18b20.getTempCByIndex(0);

  LOG_PRINT("  [Sensor] Water Temp: ");
  if (tempC == DEVICE_DISCONNECTED_C || tempC < -50 || tempC > 120) {
    LOG_PRINTLN("ERROR (disconnected or invalid reading)");
    return NAN;
  }
  LOG_PRINTF("%.2f C\n", tempC);
  return tempC;
}

/**
 * @brief Reads air temperature and humidity from the DHT22 sensor.
 * @param temp Reference to a float to store the temperature.
 * @param humidity Reference to a float to store the humidity.
 */
static void read_dht(float &temp, float &humidity) {
  humidity = dht.readHumidity();
  temp = dht.readTemperature();

  LOG_PRINT("  [Sensor] Air T/H: ");
  if (isnan(humidity) || isnan(temp)) {
    LOG_PRINTLN("ERROR (failed to read)");
    temp = NAN;
    humidity = NAN;
  } else {
    LOG_PRINTF("%.2f C, %.2f %%\n", temp, humidity);
  }
}

/**
 * @brief Reads the ultrasonic sensor to determine water distance and level.
 * @param distance Reference to a float to store the raw distance to the water.
 * @param level Reference to a float to store the calculated water level.
 */
static void read_ultrasonic(float &distance, float &level) {
  unsigned int usDistance = sonar.ping_cm();
  LOG_PRINT("  [Sensor] Ultrasonic: ");

  if (usDistance == 0 || usDistance >= ULTRASONIC_MAX_DISTANCE_CM) {
    LOG_PRINTLN("ERROR (out of range)");
    distance = NAN;
    level = NAN;
  } else {
    distance = (float)usDistance;
    level = TANDON_MAX_HEIGHT_CM - distance;
    // Clamp level to a valid range [0, TANDON_MAX_HEIGHT_CM] to prevent negative values.
    level = max(0.0f, min((float)TANDON_MAX_HEIGHT_CM, level));
    LOG_PRINTF("Dist: %.0f cm, Level: %.1f cm\n", distance, level);
  }
}

/**
 * @brief Reads the TDS sensor and calculates a temperature-compensated value.
 * @param waterTemp The current water temperature, required for compensation.
 * @return The compensated TDS value in ppm, or NAN on failure.
 */
static float read_tds(float waterTemp) {
  const float VREF = 3.3;
  const int ADC_RESOLUTION = 4095;
  float voltage = analogRead(TDS_SENSOR_PIN) * VREF / ADC_RESOLUTION;

  LOG_PRINT("  [Sensor] TDS: ");
  // Validate that the voltage is within a plausible range for the sensor.
  if (voltage > TDS_MAX_VOLTAGE || voltage < TDS_MIN_VOLTAGE) {
    LOG_PRINTF("ERROR (invalid voltage: %.2fV)\n", voltage);
    return NAN;
  }

  // Temperature compensation formula
  float rawTds = voltage * TDS_K_VALUE;
  float compensatedTds = rawTds / (1.0 + TDS_TEMP_COEFF * (waterTemp - 25.0));

  LOG_PRINTF("Voltage: %.2fV, Comp. TDS: %.1f ppm\n", voltage, compensatedTds);
  return compensatedTds;
}

/**
 * @brief Reads all values from the PZEM-004T power meter.
 * @param values Reference to the main SensorValues struct to populate.
 */
static void read_pzem(SensorValues &values) {
  LOG_PRINT("  [Sensor] PZEM-004T: ");

  // Read voltage first. The library returns NAN on failure.
  float voltage = pzem.voltage();

  if (!isnan(voltage)) {
    // If voltage is valid, read the rest of the values.
    values.pzemVoltage = voltage;
    values.pzemCurrent = pzem.current();
    values.pzemPower = pzem.power();
    values.pzemEnergy = pzem.energy() / 1000.0f; // Convert Wh to kWh for Home Assistant
    values.pzemFrequency = pzem.frequency();
    values.pzemPowerFactor = pzem.pf();

    LOG_PRINTF("V:%.1f, A:%.3f, W:%.1f\n", values.pzemVoltage, values.pzemCurrent, values.pzemPower);
  } else {
    LOG_PRINTLN("ERROR (failed to read)");
    // Set all related values to Not-a-Number to indicate failure.
    values.pzemVoltage = NAN;
    values.pzemCurrent = NAN;
    values.pzemPower = NAN;
    values.pzemEnergy = NAN;
    values.pzemFrequency = NAN;
    values.pzemPowerFactor = NAN;
  }
}

/**
 * @brief Reads the pH sensor and calculates the pH value based on a 4-point calibration.
 * Uses segmental linear interpolation for maximum accuracy across the pH range,
 * based on the data from the dedicated calibration script.
 * @return The calculated pH value, or NAN on failure.
 */
static float read_ph() {
  const float VREF = 3.3;
  const int ADC_RESOLUTION = 4095;
  
  // Read ADC multiple times and take the average to reduce noise.
  int totalADC = 0;
  const int samples = 10;
  for(int i = 0; i < samples; i++) {
    totalADC += analogRead(PH_SENSOR_PIN);
    delay(10);
  }
  int adcValue = totalADC / samples;
  float voltage = adcValue * VREF / ADC_RESOLUTION;

  LOG_PRINT("  [Sensor] pH: ");

  // Validate that the voltage is within a plausible range for the sensor.
  if (voltage < PH_MIN_VOLTAGE || voltage > PH_MAX_VOLTAGE) {
    LOG_PRINTF("ERROR (invalid voltage: %.2fV). Ensure sensor is powered by 3.3V.\n", voltage);
    return NAN;
  }

  // 4-Point Calibration using segmental linear interpolation, matching calibration script.
  float ph_value = 0.0;

  if (voltage >= PH_CALIBRATION_VOLTAGE_401) { // Extrapolation for very acidic range (pH < 4.01)
    float slope_acid = (4.01 - 6.86) / (PH_CALIBRATION_VOLTAGE_401 - PH_CALIBRATION_VOLTAGE_686);
    ph_value = 6.86 + (voltage - PH_CALIBRATION_VOLTAGE_686) * slope_acid;
  } else if (voltage >= PH_CALIBRATION_VOLTAGE_686) { // Acidic to neutral range (pH 4.01 to 6.86)
    float slope_acid = (4.01 - 6.86) / (PH_CALIBRATION_VOLTAGE_401 - PH_CALIBRATION_VOLTAGE_686);
    ph_value = 6.86 + (voltage - PH_CALIBRATION_VOLTAGE_686) * slope_acid;
  } else if (voltage >= PH_CALIBRATION_VOLTAGE_918) { // Neutral to alkaline range (pH 6.86 to 9.18)
    float slope_alkaline = (6.86 - 9.18) / (PH_CALIBRATION_VOLTAGE_686 - PH_CALIBRATION_VOLTAGE_918);
    ph_value = 9.18 + (voltage - PH_CALIBRATION_VOLTAGE_918) * slope_alkaline;
  } else { // Extrapolation for very alkaline range (pH > 9.18)
    float slope_alkaline = (6.86 - 9.18) / (PH_CALIBRATION_VOLTAGE_686 - PH_CALIBRATION_VOLTAGE_918);
    ph_value = 9.18 + (voltage - PH_CALIBRATION_VOLTAGE_918) * slope_alkaline;
  }

  // Clamp the result to a reasonable range (e.g., 0 to 14) to avoid extreme values from extrapolation.
  ph_value = max(0.0f, min(14.0f, ph_value));

  LOG_PRINTF("Voltage: %.3fV, pH: %.2f (4-point calibration)\n", voltage, ph_value);
  return ph_value;
}
