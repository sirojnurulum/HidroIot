#include "sensors.h"

#if HYDROPONIC_INSTANCE == 1

#include "config.h" // For pin definitions and sensor configurations

// Include sensor libraries
#include <NewPing.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <PZEM004Tv30.h>

// --- Module-Private (Static) Objects ---
static NewPing sonar(ULTRASONIC_TRIGGER_PIN, ULTRASONIC_ECHO_PIN, ULTRASONIC_MAX_DISTANCE_CM);
static OneWire oneWire(ONE_WIRE_BUS);
static DallasTemperature ds18b20(&oneWire);
static DHT dht(DHT_PIN, DHT_TYPE);
// Declare pzem as a pointer. It will be initialized in sensors_init() to avoid static initialization order issues.
static PZEM004Tv30* pzem = nullptr;

// --- Forward Declarations for Static Functions ---
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
  
  // Initialize the PZEM object, providing the Serial port AND the RX/TX pins.
  // This matches the library's constructor and is the correct way to instantiate it.
  pzem = new PZEM004Tv30(Serial2, PZEM_RX_PIN, PZEM_TX_PIN);
  analogReadResolution(12); // Set ADC resolution to 12-bit for TDS sensor
}

void sensors_read_all(SensorValues &values) {
  LOG_PRINTLN("\n--- Reading All Sensors ---");
  read_ultrasonic(values.waterDistanceCm, values.waterLevelCm);
  values.waterTempC = read_water_temperature();
  // Only read TDS if water temperature is valid, as it's needed for compensation
  values.tdsPpm = isnan(values.waterTempC) ? NAN : read_tds(values.waterTempC);
  read_dht(values.airTempC, values.airHumidityPercent);
  read_pzem(values);
  values.phValue = read_ph();
  LOG_PRINTLN("--- Finished Sensor Readings ---\n");
}

// --- Static (Private) Function Implementations ---

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
    // Clamp level to valid range [0, TANDON_MAX_HEIGHT_CM]
    level = max(0.0f, min((float)TANDON_MAX_HEIGHT_CM, level));
    LOG_PRINTF("Dist: %.0f cm, Level: %.1f cm\n", distance, level);
  }
}

static float read_tds(float waterTemp) {
  const float VREF = 3.3;
  const int ADC_RESOLUTION = 4095;
  float voltage = analogRead(TDS_SENSOR_PIN) * VREF / ADC_RESOLUTION;

  LOG_PRINT("  [Sensor] TDS: ");
  if (voltage > 2.4 || voltage < 0.1) { // Plausible voltage range
    LOG_PRINTF("ERROR (invalid voltage: %.2fV)\n", voltage);
    return NAN;
  }

  float rawTds = voltage * TDS_K_VALUE;
  float compensatedTds = rawTds / (1.0 + TDS_TEMP_COEFF * (waterTemp - 25.0));

  LOG_PRINTF("Voltage: %.2fV, Comp. TDS: %.1f ppm\n", voltage, compensatedTds);
  return compensatedTds;
}

static void read_pzem(SensorValues &values) {
  LOG_PRINT("  [Sensor] PZEM-004T: ");

  // Safety check to ensure the pzem object was created successfully.
  if (!pzem) {
    LOG_PRINTLN("ERROR (PZEM object not initialized)");
    values.pzemVoltage = NAN;
    values.pzemCurrent = NAN;
    values.pzemPower = NAN;
    values.pzemEnergy = NAN;
    values.pzemFrequency = NAN;
    values.pzemPowerFactor = NAN;
    return;
  }

  // Read each value individually using the arrow operator for pointers.
  // The library returns NAN on failure.
  float voltage = pzem->voltage();

  if (!isnan(voltage)) {
    // If voltage is valid, read the rest of the values
    values.pzemCurrent = pzem->current();
    values.pzemPower = pzem->power();
    values.pzemEnergy = pzem->energy() / 1000.0f; // Convert Wh to kWh
    values.pzemFrequency = pzem->frequency();
    values.pzemPowerFactor = pzem->pf();
    values.pzemVoltage = voltage;

    LOG_PRINTF("V:%.1f, A:%.3f, W:%.1f\n", values.pzemVoltage, values.pzemCurrent, values.pzemPower);
  } else {
    LOG_PRINTLN("ERROR (failed to read)");
    // Set all values to Not-a-Number to indicate failure
    values.pzemVoltage = NAN;
    values.pzemCurrent = NAN;
    values.pzemPower = NAN;
    values.pzemEnergy = NAN;
    values.pzemFrequency = NAN;
    values.pzemPowerFactor = NAN;
  }
}

static float read_ph() {
  const float VREF = 3.3;
  const int ADC_RESOLUTION = 4095;
  int adcValue = analogRead(PH_SENSOR_PIN);
  float voltage = adcValue * VREF / ADC_RESOLUTION;

  LOG_PRINT("  [Sensor] pH: ");

  // Check for invalid voltage (e.g., sensor disconnected)
  if (voltage < 0.1 || voltage > 3.2) { // Relaxed upper bound, ESP32 ADC max is 3.3V
    LOG_PRINTF("ERROR (invalid voltage: %.2fV). Ensure sensor is powered by 3.3V.\n", voltage);
    return NAN;
  }

  // Linear interpolation/extrapolation based on two-point calibration
  // Formula: y = y1 + ((x - x1) / (x2 - x1)) * (y2 - y1)
  // (x1, y1) = (voltage at pH 7, 7.0)
  // (x2, y2) = (voltage at pH 4, 4.0)
  float phValue = 7.0 + ((voltage - PH_CALIBRATION_VOLTAGE_7) / (PH_CALIBRATION_VOLTAGE_4 - PH_CALIBRATION_VOLTAGE_7)) * (4.0 - 7.0);

  LOG_PRINTF("Voltage: %.2fV, pH: %.2f\n", voltage, phValue);
  return phValue;
}

#endif // HYDROPONIC_INSTANCE == 1
