// =======================================================================
//                           LIBRARY IMPORTS
// =======================================================================
#include <Arduino.h>
#include <WiFi.h>         // Untuk koneksi Wi-Fi
#include <PubSubClient.h> // Untuk MQTT
#include <NewPing.h>      // Untuk sensor ultrasonik JSN-SR04T

// Library untuk DS18B20 (Suhu Air)
#include <OneWire.h>
#include <DallasTemperature.h>

// Library untuk DHT Sensor (Suhu & Kelembaban Udara)
#include <Adafruit_Sensor.h> // Library dasar Adafruit untuk sensor
#include <DHT.h>             // Library untuk sensor DHT

// =======================================================================
//                           WIFI CONFIGURATION
// =======================================================================
const char *WIFI_SSID = "Baceprot";                      // SSID WiFi Anda
const char *WIFI_PASSWORD = "ya.gak.tau.kok.tanya.saya"; // Password WiFi Anda

// =======================================================================
//                           MQTT CONFIGURATION
// =======================================================================
const char *MQTT_SERVER = "ha.o-m-b.id"; // Domain MQTT Broker Anda
const int MQTT_PORT = 1883;
const char *MQTT_USERNAME = "Baceprot";
const char *MQTT_PASSWORD = "Baceprot@IotHidro";
const char *MQTT_CLIENT_ID = "esp32hidro"; // ID unik untuk perangkat ini di MQTT

// =======================================================================
//                           PIN DEFINITIONS
// =======================================================================
// Sensor Ultrasonik JSN-SR04T
#define ULTRASONIC_TRIGGER_PIN 5
#define ULTRASONIC_ECHO_PIN 4
#define ULTRASONIC_MAX_DISTANCE_CM 200 // Jarak maksimal sensor dalam cm

// Sensor Suhu DS18B20 (Suhu Air)
#define ONE_WIRE_BUS 16 // Pin GPIO yang terhubung ke data pin DS18B20

// Sensor Suhu & Kelembaban DHT22 (Suhu & Kelembaban Udara)
#define DHT_PIN 13     // Pin GPIO yang terhubung ke pin DA/Data pada sensor DHT
#define DHT_TYPE DHT22 // Menggunakan DHT22 (AM2302)

// Buzzer Peringatan
const int BUZZER_PIN = 17; // Pin GPIO untuk Buzzer

// Pompa Peristaltik (Relay/Modul MOSFET HIGH LEVEL TRIGGER: HIGH = ON, LOW = OFF)
#define PUMP_NUTRISI_A_PIN 25
#define PUMP_NUTRISI_B_PIN 26
#define PUMP_PH_PIN 27

// =======================================================================
//                           SYSTEM CONFIGURATION
// =======================================================================
#define TANDON_MAX_HEIGHT_CM 100 // Tinggi tandon air Anda dalam cm

// Interval Waktu (dalam milidetik)
const long SENSOR_PUBLISH_INTERVAL_MS = 5000; // Kirim data sensor setiap 5 detik
const long HEARTBEAT_INTERVAL_MS = 10000;     // Kirim heartbeat setiap 10 detik
const long WIFI_RECONNECT_DELAY_MS = 5000;    // Delay jika gagal konek WiFi
const long MQTT_RECONNECT_DELAY_MS = 5000;    // Delay jika gagal konek MQTT
const int MAX_RECONNECT_ATTEMPTS = 60;        // Jumlah maksimum percobaan koneksi ulang

// =======================================================================
//                           MQTT TOPICS
// =======================================================================
// Status Sensor
const char *STATE_TOPIC_LEVEL = "hidroponik/air/level_cm";               // Level air (cm)
const char *STATE_TOPIC_DISTANCE = "hidroponik/air/jarak_sensor_cm";     // Jarak sensor ke air (cm)
const char *STATE_TOPIC_WATER_TEMPERATURE = "hidroponik/air/suhu_c";     // Suhu air (°C)
const char *STATE_TOPIC_AIR_TEMPERATURE = "hidroponik/udara/suhu_c";     // Suhu udara (°C)
const char *STATE_TOPIC_HUMIDITY = "hidroponik/udara/kelembaban_persen"; // Kelembaban udara (%)

// Status Sistem
const char *AVAILABILITY_TOPIC = "tele/esp32hidro/LWT";        // LWT (Last Will and Testament) untuk status online/offline
const char *HEARTBEAT_TOPIC = "tele/esp32hidro/HEARTBEART";    // Heartbeat untuk indikasi ESP32 masih hidup
const char *MQTT_GLOBAL_ALERT_TOPIC = "hidroponik/peringatan"; // Topik khusus untuk semua peringatan

// Kontrol & Status Pompa Peristaltik
const char *COMMAND_TOPIC_PUMP_A = "hidroponik/pompa/nutrisi_a/kontrol"; // Kontrol Pompa Nutrisi A
const char *STATE_TOPIC_PUMP_A = "hidroponik/pompa/nutrisi_a/status";    // Status Pompa Nutrisi A

const char *COMMAND_TOPIC_PUMP_B = "hidroponik/pompa/nutrisi_b/kontrol"; // Kontrol Pompa Nutrisi B
const char *STATE_TOPIC_PUMP_B = "hidroponik/pompa/nutrisi_b/status";    // Status Pompa Nutrisi B

const char *COMMAND_TOPIC_PUMP_PH = "hidroponik/pompa/ph/kontrol"; // Kontrol Pompa pH
const char *STATE_TOPIC_PUMP_PH = "hidroponik/pompa/ph/status";    // Status Pompa pH

// =======================================================================
//                           GLOBAL VARIABLES & OBJECTS
// =======================================================================
WiFiClient espClient;
PubSubClient mqttClient(espClient);
NewPing sonar(ULTRASONIC_TRIGGER_PIN, ULTRASONIC_ECHO_PIN, ULTRASONIC_MAX_DISTANCE_CM);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DHT dht(DHT_PIN, DHT_TYPE);

// Batas Ambang Peringatan (Sesuaikan sesuai kebutuhan Anda)
const float WATER_LEVEL_CRITICAL_CM = 20.0; // Jika air <= 20 cm, beri peringatan

// Variabel untuk waktu terakhir publikasi/aksi
unsigned long lastSensorPublishTime = 0;
unsigned long lastHeartbeatTime = 0;

// Status pompa peristaltik (true = ON, false = OFF)
bool isPumpAOn = false;
bool isPumpBOn = false;
bool isPumpPHOn = false;

// Variabel global untuk menyimpan nilai sensor terbaru
float currentWaterLevel = 0.0;
float currentWaterTemp = 0.0;
float currentAirTemp = 0.0;
float currentHumidity = 0.0;

// Status Peringatan (true = aktif, false = tidak aktif)
bool isWaterLevelAlertActive = false; // Status apakah peringatan level air sedang aktif

// =======================================================================
//                           FUNCTION DECLARATIONS
// =======================================================================
void setupWifi();
void reconnectMqtt();
void mqttCallback(char *topic, byte *payload, unsigned int length);
bool publishData(const char *topic, const char *payload, bool retain);
void sendAlert(const char *alertMessage);

// Fungsi pembacaan sensor
float readWaterTemperature();
void readAndPublishUltrasonicData();
void readAndPublishDHTData();

// Fungsi kontrol aktuator
void handlePumpCommand(const String &pumpTopic, const String &command);
void updateBuzzerAlert();

// =======================================================================
//                           FUNCTION IMPLEMENTATIONS
// =======================================================================

// Fungsi untuk koneksi Wi-Fi
void setupWifi()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    return; // Sudah terhubung, tidak perlu konek lagi
  }
  Serial.print("\nConnecting to WiFi ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < MAX_RECONNECT_ATTEMPTS)
  {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println(
        "\nFailed to connect to WiFi after " + String(MAX_RECONNECT_ATTEMPTS) +
        " attempts. Restarting ESP32 in 5 seconds...");
    delay(5000);
    ESP.restart(); // Restart ESP32 jika gagal konek WiFi
  }
}

// Callback untuk menerima pesan MQTT
void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("\n[MQTT Command] Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Payload: ");
  String messagePayload;
  for (unsigned int i = 0; i < length; i++)
  {
    messagePayload += (char)payload[i];
  }
  Serial.println(messagePayload);

  // Penanganan perintah pompa
  if (String(topic) == COMMAND_TOPIC_PUMP_A ||
      String(topic) == COMMAND_TOPIC_PUMP_B ||
      String(topic) == COMMAND_TOPIC_PUMP_PH)
  {
    handlePumpCommand(String(topic), messagePayload);
  }
}

// Fungsi untuk koneksi ulang MQTT
void reconnectMqtt()
{
  int attempts = 0;
  while (!mqttClient.connected() && WiFi.status() == WL_CONNECTED && attempts < MAX_RECONNECT_ATTEMPTS)
  {
    Serial.print("[MQTT] Attempting MQTT connection...");
    // Menggunakan user dan password untuk koneksi MQTT
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD,
                           AVAILABILITY_TOPIC, 0, true, "Offline"))
    {
      Serial.println("connected");
      if (mqttClient.connected())
      {
        publishData(AVAILABILITY_TOPIC, "Online", true);
        // Publikasikan status awal pompa dan subscribe ke topik kontrol
        publishData(STATE_TOPIC_PUMP_A, isPumpAOn ? "ON" : "OFF", true);
        publishData(STATE_TOPIC_PUMP_B, isPumpBOn ? "ON" : "OFF", true);
        publishData(STATE_TOPIC_PUMP_PH, isPumpPHOn ? "ON" : "OFF", true);

        mqttClient.subscribe(COMMAND_TOPIC_PUMP_A);
        mqttClient.subscribe(COMMAND_TOPIC_PUMP_B);
        mqttClient.subscribe(COMMAND_TOPIC_PUMP_PH);
        Serial.println("[MQTT] Subscribed to pump control topics.");
      }
    }
    else
    {
      Serial.print("[MQTT] failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" trying again in 5 seconds");
      delay(MQTT_RECONNECT_DELAY_MS);
    }
    attempts++;
  }
  if (!mqttClient.connected() && WiFi.status() == WL_CONNECTED)
  {
    Serial.println(
        "[MQTT] Failed to connect to MQTT after " + String(MAX_RECONNECT_ATTEMPTS) +
        " attempts. Sensor readings will continue, but data will not be published to MQTT.");
  }
}

// Fungsi bantu untuk publikasi MQTT (dengan 5 spasi di awal)
bool publishData(const char *topic, const char *payload, bool retain)
{
  if (!mqttClient.connected())
  {
    Serial.print("     [PUB] MQTT Client not connected. Skipping publish to ");
    Serial.println(topic);
    return false;
  }
  Serial.print("     [PUB] Publishing to ");
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(payload);

  return mqttClient.publish(topic, payload, retain);
}

// Fungsi bantu untuk mengirim pesan peringatan ke topik khusus
void sendAlert(const char *alertMessage)
{
  publishData(MQTT_GLOBAL_ALERT_TOPIC, alertMessage, false);
}

// Membaca suhu dari sensor DS18B20 untuk air
float readWaterTemperature()
{
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  char payloadBuffer[10];

  Serial.print("[Sensor] Water Temp: ");
  if (tempC == DEVICE_DISCONNECTED_C || tempC < -50 || tempC > 120)
  {
    Serial.println("Error: Sensor disconnected or invalid reading!");
    currentWaterTemp = NAN;
    publishData(STATE_TOPIC_WATER_TEMPERATURE, "unavailable", false);
    return NAN;
  }
  currentWaterTemp = tempC;
  Serial.printf("%.2f °C\n", tempC);

  dtostrf(tempC, 5, 2, payloadBuffer);
  publishData(STATE_TOPIC_WATER_TEMPERATURE, payloadBuffer, false);

  return tempC;
}

// Membaca dan Mempublikasikan Data Ultrasonik (Logika Diperbarui)
void readAndPublishUltrasonicData()
{
  unsigned int usDistance = sonar.ping_cm();
  float waterLevelCm = 0;
  char payloadBuffer[10];

  Serial.print("[Sensor] Ultrasonic - ");

  if (usDistance == 0 || usDistance >= ULTRASONIC_MAX_DISTANCE_CM)
  {
    Serial.println("Water Level: Out of Range / No Echo");
    publishData(STATE_TOPIC_LEVEL, "unavailable", false);
    publishData(STATE_TOPIC_DISTANCE, "unavailable", false);
    currentWaterLevel = -1.0; // Menandakan tidak valid

    if (!isWaterLevelAlertActive)
    {
      isWaterLevelAlertActive = true;
      sendAlert("ALERT: Level air tidak terdeteksi atau di luar jangkauan!");
      Serial.println(">>> Peringatan: Level air tidak terdeteksi! <<<");
    }
  }
  else
  {
    // LOGIKA BARU SESUAI PERMINTAAN PENGGUNA:
    // Jika jarak yang terukur lebih besar dari tinggi maksimum tandon,
    // maka asumsikan tandon kosong (level air 0).
    if (usDistance > TANDON_MAX_HEIGHT_CM)
    {
      waterLevelCm = 0;
      Serial.printf("Distance: %d cm (beyond max height), Level: %.1f cm (empty)\n", usDistance, waterLevelCm);
    }
    else
    {
      // Perhitungan normal: Tinggi tandon - Jarak ke permukaan air
      waterLevelCm = TANDON_MAX_HEIGHT_CM - usDistance;

      // Batasi level air agar tidak melebihi tinggi tandon (jika sensor terendam/pembacaan terlalu rendah)
      if (waterLevelCm > TANDON_MAX_HEIGHT_CM)
      {
        waterLevelCm = TANDON_MAX_HEIGHT_CM;
        Serial.printf("Distance: %d cm, Level: %.1f cm (clamped to full)\n", usDistance, waterLevelCm);
      }
      else
      {
        Serial.printf("Distance: %d cm, Level: %.1f cm\n", usDistance, waterLevelCm);
      }
    }

    dtostrf((float)usDistance, 4, 0, payloadBuffer);
    publishData(STATE_TOPIC_DISTANCE, payloadBuffer, false);

    dtostrf(waterLevelCm, 4, 1, payloadBuffer);
    publishData(STATE_TOPIC_LEVEL, payloadBuffer, false);
    currentWaterLevel = waterLevelCm;

    // Cek Kondisi Peringatan Level Air
    if (currentWaterLevel <= WATER_LEVEL_CRITICAL_CM)
    {
      if (!isWaterLevelAlertActive)
      {
        isWaterLevelAlertActive = true;
        char alertMessage[50];
        sprintf(alertMessage, "ALERT: Level air kritis! %.1f cm", currentWaterLevel);
        sendAlert(alertMessage);
        Serial.println(">>> Peringatan: Level air kritis! <<<");
      }
    }
    else
    {
      if (isWaterLevelAlertActive)
      {
        isWaterLevelAlertActive = false;
        sendAlert("Level air normal kembali.");
        Serial.println("Level air kembali normal.");
      }
    }
  }
}

// Fungsi untuk membaca dan mempublikasikan data DHT (Suhu & Kelembaban Udara)
void readAndPublishDHTData()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // Read temperature as Celsius (the default)

  Serial.print("[Sensor] Air Temp/Humidity: ");
  if (isnan(h) || isnan(t))
  {
    Serial.println("Error: Failed to read from DHT sensor!");
    publishData(STATE_TOPIC_AIR_TEMPERATURE, "unavailable", false);
    publishData(STATE_TOPIC_HUMIDITY, "unavailable", false);
    currentAirTemp = NAN;
    currentHumidity = NAN;
    return;
  }

  currentAirTemp = t;
  currentHumidity = h;
  Serial.printf("Temp: %.2f °C, Humidity: %.2f %%\n", t, h);
  char payloadBuffer[10];

  dtostrf(t, 5, 2, payloadBuffer);
  publishData(STATE_TOPIC_AIR_TEMPERATURE, payloadBuffer, false);

  dtostrf(h, 5, 2, payloadBuffer);
  publishData(STATE_TOPIC_HUMIDITY, payloadBuffer, false);
}

// Fungsi untuk menangani perintah ON/OFF untuk setiap pompa
void handlePumpCommand(const String &pumpTopic, const String &command)
{
  int pumpPin;
  bool *pumpStatus;
  const char *stateTopic;
  String pumpName;

  if (pumpTopic == COMMAND_TOPIC_PUMP_A)
  {
    pumpPin = PUMP_NUTRISI_A_PIN;
    pumpStatus = &isPumpAOn;
    stateTopic = STATE_TOPIC_PUMP_A;
    pumpName = "Nutrisi A";
  }
  else if (pumpTopic == COMMAND_TOPIC_PUMP_B)
  {
    pumpPin = PUMP_NUTRISI_B_PIN;
    pumpStatus = &isPumpBOn;
    stateTopic = STATE_TOPIC_PUMP_B;
    pumpName = "Nutrisi B";
  }
  else if (pumpTopic == COMMAND_TOPIC_PUMP_PH)
  {
    pumpPin = PUMP_PH_PIN;
    pumpStatus = &isPumpPHOn;
    stateTopic = STATE_TOPIC_PUMP_PH;
    pumpName = "pH";
  }
  else
  {
    Serial.println("[Pump Control] Invalid pump topic received.");
    return;
  }

  if (command == "ON")
  {
    digitalWrite(pumpPin, HIGH); // HIGH = ON untuk modul Anda (HIGH Level Trigger)
    *pumpStatus = true;
    Serial.print("[Pump Control] Pump ");
    Serial.print(pumpName);
    Serial.println(" turned ON");
    publishData(stateTopic, "ON", true);
  }
  else if (command == "OFF")
  {
    digitalWrite(pumpPin, LOW); // LOW = OFF untuk modul Anda (HIGH Level Trigger)
    *pumpStatus = false;
    Serial.print("[Pump Control] Pump ");
    Serial.print(pumpName);
    Serial.println(" turned OFF");
    publishData(stateTopic, "OFF", true);
  }
  else
  {
    Serial.println("[Pump Control] Invalid pump command received: " + command);
  }
}

// Fungsi untuk Mengelola Peringatan Buzzer
void updateBuzzerAlert()
{
  // Buzzer akan aktif hanya jika peringatan level air aktif
  if (isWaterLevelAlertActive)
  {
    if (digitalRead(BUZZER_PIN) == LOW)
    {                                 // Cek apakah buzzer belum aktif
      digitalWrite(BUZZER_PIN, HIGH); // Aktifkan buzzer
      Serial.println(">>> BUZZER ON: Peringatan aktif! <<<");
    }
  }
  else
  {
    if (digitalRead(BUZZER_PIN) == HIGH)
    {                                // Cek apakah buzzer masih aktif
      digitalWrite(BUZZER_PIN, LOW); // Matikan buzzer
      Serial.println(">>> BUZZER OFF: Tidak ada peringatan aktif. <<<");
    }
  }
}

// =======================================================================
//                           SETUP FUNCTION
// =======================================================================
void setup()
{
  Serial.begin(115200);
  Serial.println("--- ESP32 Hydroponic Automation System ---");
  Serial.println("Inisialisasi sistem...");

  // Inisialisasi pin pompa sebagai OUTPUT dan pastikan OFF saat startup
  pinMode(PUMP_NUTRISI_A_PIN, OUTPUT);
  pinMode(PUMP_NUTRISI_B_PIN, OUTPUT);
  pinMode(PUMP_PH_PIN, OUTPUT);
  digitalWrite(PUMP_NUTRISI_A_PIN, LOW); // LOW = OFF untuk modul Anda
  digitalWrite(PUMP_NUTRISI_B_PIN, LOW);
  digitalWrite(PUMP_PH_PIN, LOW);
  isPumpAOn = false; // Set status awal
  isPumpBOn = false;
  isPumpPHOn = false;

  // Inisialisasi pin Buzzer dan pastikan mati
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Inisialisasi sensor OneWire (DS18B20)
  sensors.begin();

  // Inisialisasi sensor DHT
  dht.begin();

  // Coba konek WiFi
  setupWifi();

  // Setup MQTT Client
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);

  Serial.println("Setup Complete. Memulai Loop...");
}

// =======================================================================
//                           LOOP FUNCTION
// =======================================================================
void loop()
{
  unsigned long currentTime = millis(); // Ambil waktu saat ini

  // Cek koneksi Wi-Fi dan sambungkan kembali jika putus
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("[WiFi] Disconnected! Attempting to reconnect...");
    setupWifi();
  }

  // Cek koneksi MQTT dan sambungkan kembali jika putus (hanya jika WiFi terhubung)
  if (!mqttClient.connected() && WiFi.status() == WL_CONNECTED)
  {
    reconnectMqtt();
  }
  mqttClient.loop(); // Penting: Memproses pesan MQTT masuk dan mempertahankan koneksi

  // Baca dan publikasikan data sensor secara berkala
  if (currentTime - lastSensorPublishTime >= SENSOR_PUBLISH_INTERVAL_MS)
  {
    lastSensorPublishTime = currentTime;

    Serial.println("\n--- START SENSOR READINGS & MQTT PUBLICATION CYCLE ---");

    readAndPublishUltrasonicData();
    readWaterTemperature();
    readAndPublishDHTData();

    // Perbarui status buzzer berdasarkan nilai sensor yang diukur
    updateBuzzerAlert();

    Serial.println("--- END SENSOR READINGS & MQTT PUBLICATION CYCLE ---\n");
  }

  // Publikasikan heartbeat secara berkala (hanya jika terhubung MQTT)
  if (currentTime - lastHeartbeatTime >= HEARTBEAT_INTERVAL_MS)
  {
    lastHeartbeatTime = currentTime;
    if (mqttClient.connected())
    {
      Serial.println("--- START HEARTBEAT ---");
      publishData(HEARTBEAT_TOPIC, "ESP32 Online", true);
      Serial.println("--- END HEARTBEAT ---");
    }
    else
    {
      Serial.println("[Heartbeat] Skipping heartbeat: MQTT not connected.");
    }
  }

  delay(50); // Delay singkat untuk stabilitas
}
