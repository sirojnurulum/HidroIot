#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h> // For isnan()

#if HYDROPONIC_INSTANCE == 1
/**
 * @struct SensorValues
 * @brief A single structure to hold all sensor readings, making it easy to pass data between modules.
 *        Struktur tunggal untuk menampung semua pembacaan sensor, memudahkan pengiriman data antar modul.
 */
struct SensorValues {
  float waterLevelCm;
  float waterDistanceCm;
  float waterTempC;
  float airTempC;
  float airHumidityPercent;
  float tdsPpm;
};

/**
 * @brief Menginisialisasi semua sensor yang terhubung. Panggil fungsi ini sekali di dalam setup().
 * 
 * @brief Initializes all connected sensors. Call this once in setup().
 */
void sensors_init();

/**
 * @brief Membaca semua sensor dan mengisi struct SensorValues yang diberikan.
 * 
 * @brief Reads all sensors and populates the provided SensorValues struct.
 * @param values Referensi ke struct SensorValues untuk diisi dengan data. / Reference to the SensorValues struct to be filled with data.
 */
void sensors_read_all(SensorValues &values);

#endif // HYDROPONIC_INSTANCE == 1

#endif // SENSORS_H
