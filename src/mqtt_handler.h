#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <PubSubClient.h>
#include "sensors.h" // For SensorValues struct

/**
 * @brief Menginisialisasi klien MQTT dan mengatur fungsi callback.
 * 
 * @brief Initializes the MQTT client and sets the callback function.
 */
void mqtt_init();

/**
 * @brief Menangani koneksi MQTT dan memproses pesan masuk. Panggil fungsi ini di dalam loop() utama.
 * 
 * @brief Handles MQTT connection and processes incoming messages. Call this in the main loop().
 */
void mqtt_loop();

/**
 * @brief Mempublikasikan semua data sensor dari struct yang diberikan ke topik masing-masing.
 * 
 * @brief Publishes all sensor data from the provided struct to their respective topics.
 * @param values Struct SensorValues yang berisi data sensor terbaru. / The SensorValues struct containing the latest sensor data.
 */
void mqtt_publish_sensor_data(const SensorValues &values);

/**
 * @brief Mempublikasikan pesan heartbeat untuk menandakan perangkat sedang online.
 * 
 * @brief Publishes a heartbeat message to indicate the device is online.
 */
void mqtt_publish_heartbeat();

/**
 * @brief Mempublikasikan pesan peringatan generik ke topik peringatan global.
 * 
 * @brief Publishes a generic alert message to the global alert topic.
 * @param alertMessage Pesan peringatan yang akan dikirim. / The alert message to be sent.
 */
void mqtt_publish_alert(const char* alertMessage);

/**
 * @brief Mempublikasikan pesan status generik ke sebuah topik. Digunakan oleh modul aktuator.
 * 
 * @brief Publishes a generic state message to a topic. This will be used by the actuators module.
 * @param topic Topik MQTT tujuan. / The destination MQTT topic.
 * @param payload Pesan yang akan dikirim. / The message to be sent.
 * @param retain Apakah pesan harus dipertahankan (retained) oleh broker. / Whether the message should be retained by the broker.
 */
void mqtt_publish_state(const char* topic, const char* payload, bool retain);

/**
 * @brief Mengembalikan true jika klien MQTT sedang terhubung.
 * 
 * @brief Returns true if the MQTT client is currently connected.
 */
bool mqtt_is_connected();

#endif // MQTT_HANDLER_H
