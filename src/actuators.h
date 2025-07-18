#ifndef ACTUATORS_H
#define ACTUATORS_H

#if HYDROPONIC_INSTANCE == 1
#include "sensors.h" // For SensorValues struct
#endif
#include <Arduino.h> // For String

/**
 * @brief Menginisialisasi semua pin aktuator (pompa, buzzer).
 * 
 * @brief Initializes all actuator pins (pumps, buzzer).
 */
void actuators_init();

/**
 * @brief Menangani operasi berwaktu dari aktuator. Panggil fungsi ini di dalam loop() utama.
 *        Misalnya, untuk menghentikan pompa setelah durasi yang ditentukan.
 * 
 * @brief Handles the timed operation of actuators. Call this in the main loop().
 *        For example, to stop a pump after a specified duration.
 */
void actuators_loop();

/**
 * @brief Memproses perintah MQTT yang masuk untuk mengontrol pompa.
 *        Mendukung perintah volume (misalnya, "50") dan "OFF".
 * 
 * @brief Processes an incoming MQTT command to control a pump.
 *        Supports volume commands (e.g., "50") and "OFF".
 */
void actuators_handle_pump_command(const char* topic, const String& command);

#if HYDROPONIC_INSTANCE == 1
/**
 * @brief Memproses perintah MQTT yang masuk untuk mengubah mode sistem.
 *        Mendukung mode "NUTRITION" dan "CLEANER".
 * 
 * @brief Processes an incoming MQTT command to change the system mode.
 *        Supports "NUTRITION" and "CLEANER" modes.
 */
void actuators_handle_mode_command(const String& command);

/**
 * @brief Memperbarui status peringatan (buzzer dan peringatan MQTT) berdasarkan nilai sensor.
 * 
 * @brief Updates the alert status (buzzer and MQTT alert) based on sensor values.
 * @param values Struct SensorValues yang berisi data sensor terbaru. / The SensorValues struct containing the latest sensor data.
 */
void actuators_update_alert_status(const SensorValues& values);

/**
 * @brief Memeriksa nilai sensor dan melakukan dosis nutrisi otomatis jika diperlukan.
 * 
 * @brief Checks sensor values and performs automatic nutrient dosing if required.
 * @param values Struct SensorValues yang berisi data sensor terbaru. / The SensorValues struct containing the latest sensor data.
 */
void actuators_auto_dose_nutrients(const SensorValues& values);
#endif

/**
 * @brief Mempublikasikan status saat ini dari semua pompa ke MQTT.
 *        Biasanya dipanggil saat koneksi MQTT berhasil dibuat.
 * 
 * @brief Publishes the current state of all pumps to MQTT.
 *        Typically called when an MQTT connection is successfully established.
 */
void actuators_publish_states();

#endif // ACTUATORS_H
