#include "sensors.h"
#include "config.h" // For pin definitions and sensor configurations

// Include sensor libraries
#include <NewPing.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Sensor Objects
// Objek Sensor
// Variabel ini sekarang bersifat lokal untuk file ini (static), tidak global di main.cpp
// These are now local to this file (static), not global in main.cpp
static NewPing sonar(ULTRASONIC_TRIGGER_PIN, ULTRASONIC_ECHO_PIN, ULTRASONIC_MAX_DISTANCE_CM);
static OneWire oneWire(ONE_WIRE_BUS);
static DallasTemperature ds18b20(&oneWire);
static DHT dht(DHT_PIN, DHT_TYPE);

/**
 * @brief Menginisialisasi semua library sensor dan mengatur resolusi ADC.
 * 
 * @brief Initializes all sensor libraries and sets the ADC resolution.
 */
void sensors_init() {
  LOG_PRINTLN("Initializing sensors...");
  ds18b20.begin();
  dht.begin();
  // Set the resolution of the ADC to 12 bits (0-4095) for the TDS sensor.
  // This is the default for ESP32, but it's good practice to set it explicitly.
  // Atur resolusi ADC ke 12 bit (0-4095) untuk sensor TDS.
  // Ini adalah default untuk ESP32, tetapi merupakan praktik yang baik untuk mengaturnya secara eksplisit.
  analogReadResolution(12);
}

/**
 * @brief Fungsi pembantu privat untuk membaca suhu air dari sensor DS18B20.
 * 
 * @brief Private helper function to read water temperature from the DS18B20 sensor.
 * @return float Suhu dalam Celsius, atau NAN jika gagal. / Temperature in Celsius, or NAN on failure.
 */
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

/**
 * @brief Fungsi pembantu privat untuk membaca suhu dan kelembaban udara dari sensor DHT.
 * 
 * @brief Private helper function to read air temperature and humidity from the DHT sensor.
 * @param temp Referensi untuk menyimpan nilai suhu. / Reference to store the temperature value.
 * @param humidity Referensi untuk menyimpan nilai kelembaban. / Reference to store the humidity value.
 */
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

/**
 * @brief Fungsi pembantu privat untuk membaca sensor ultrasonik dan menghitung jarak & level air.
 * 
 * @brief Private helper function to read the ultrasonic sensor and calculate distance & water level.
 * @param distance Referensi untuk menyimpan jarak dari sensor ke air. / Reference to store the distance from sensor to water.
 * @param level Referensi untuk menyimpan ketinggian level air. / Reference to store the water level height.
 */
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
        level = TANDON_MAX_HEIGHT_CM; // Clamp to full
      }
      LOG_PRINTF("Distance: %d cm, Level: %.1f cm\n", usDistance, level);
    }
  }
}

/**
 * @brief Fungsi pembantu privat untuk membaca sensor TDS.
 *        Mengonversi tegangan analog ke PPM dan menerapkan kompensasi suhu.
 * 
 * @brief Private helper function to read the TDS sensor.
 *        Converts analog voltage to PPM and applies temperature compensation.
 * @param waterTemp Suhu air saat ini untuk kompensasi. / The current water temperature for compensation.
 * @return float Nilai TDS dalam PPM, atau NAN jika gagal. / TDS value in PPM, or NAN on failure.
 */
static float read_tds(float waterTemp) {
  // The sensor gives a 0-2.3V output for a 0-1000ppm range.
  // ESP32 ADC is 12-bit (0-4095) and reads 0-3.3V by default.
  const float VREF = 3.3;
  const int ADC_RESOLUTION = 4095;

  // Read the analog value
  // Baca nilai analog
  int adcValue = analogRead(TDS_SENSOR_PIN);
  // Convert the analog value to a voltage
  // Konversi nilai analog ke tegangan
  float voltage = adcValue * VREF / ADC_RESOLUTION;

  LOG_PRINT("[Sensor] TDS - ");

  // Check if the reading is valid
  // Periksa apakah pembacaan valid
  if (voltage > 2.4) { // A little buffer over the 2.3V max
    LOG_PRINTLN("Error: Invalid voltage reading!");
    return NAN;
  }

  // Convert voltage to a raw TDS value (linear mapping based on spec)
  // Konversi tegangan ke nilai TDS mentah menggunakan faktor-K kalibrasi
  float rawTds = voltage * TDS_K_VALUE;

  // Apply temperature compensation
  // Terapkan kompensasi suhu
  float compensatedTds = rawTds / (1.0 + TDS_TEMP_COEFF * (waterTemp - 25.0));

  LOG_PRINTF("Voltage: %.2fV, Compensated TDS: %.2f ppm\n", voltage, compensatedTds);
  return compensatedTds;
}

/**
 * @brief Membaca semua sensor dan mengisi struct SensorValues.
 *        Ini adalah fungsi publik utama untuk modul sensor.
 * 
 * @brief Reads all sensors and populates the SensorValues struct.
 *        This is the main public function for the sensor module.
 * @param values Referensi ke struct SensorValues untuk diisi dengan data. / Reference to the SensorValues struct to be filled with data.
 */
void sensors_read_all(SensorValues &values) {
  LOG_PRINTLN("\n--- START SENSOR READINGS ---");
  read_ultrasonic(values.waterDistanceCm, values.waterLevelCm);
  values.waterTempC = read_water_temperature();
  // Read TDS, but only if we have a valid water temperature for compensation.
  // Baca TDS, tetapi hanya jika ada suhu air yang valid untuk kompensasi.
  values.tdsPpm = isnan(values.waterTempC) ? NAN : read_tds(values.waterTempC);
  read_dht(values.airTempC, values.airHumidityPercent);
  LOG_PRINTLN("--- END SENSOR READINGS ---\n");
}