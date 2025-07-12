#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

/**
 * @brief Struct untuk menampung semua pembacaan sensor di satu tempat.
 *        Memudahkan untuk mengirim data sensor antar modul.
 * 
 * @brief A struct to hold all sensor readings in one place.
 *        This makes it easy to pass sensor data between modules.
 */
struct SensorValues {
  float waterLevelCm;       // Ketinggian air dalam cm / Water level in cm
  float waterDistanceCm;    // Jarak sensor ke permukaan air dalam cm / Distance from sensor to water surface in cm
  float waterTempC;         // Suhu air dalam Celsius / Water temperature in Celsius
  float airTempC;           // Suhu udara dalam Celsius / Air temperature in Celsius
  float airHumidityPercent; // Kelembaban udara dalam persen / Air humidity in percent
  float tdsPpm;             // Total Dissolved Solids dalam ppm / Total Dissolved Solids in ppm
};

/**
 * @brief Menginisialisasi semua sensor. Panggil fungsi ini di dalam setup().
 * 
 * @brief Initializes all sensors. Call this in setup().
 */
void sensors_init();

/**
 * @brief Membaca semua sensor dan mengisi struct yang diberikan dengan nilai terbaru.
 *        Pembacaan yang tidak valid akan direpresentasikan dengan NAN (Not a Number).
 * 
 * @brief Reads all sensors and populates the provided struct with the latest values.
 *        Invalid readings will be represented by NAN (Not a Number).
 */
void sensors_read_all(SensorValues &values);

#endif // SENSORS_H
