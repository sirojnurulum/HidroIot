# Proyek Sistem Otomatisasi Hidroponik ESP32

*(Ganti dengan gambar proyek Anda yang sebenarnya)*

Proyek ini adalah sistem otomatisasi untuk budidaya hidroponik menggunakan ESP32, MQTT, dan terintegrasi dengan Home Assistant. Sistem ini memantau parameter penting seperti level air, suhu air, suhu udara, kelembaban, dan Total Dissolved Solids (TDS). Selain itu, sistem ini menyediakan kontrol presisi berbasis volume untuk pompa nutrisi dan pH, serta sistem peringatan dini untuk level air kritis.

## Daftar Isi

*   [Fitur Utama](#fitur-utama)
*   [Daftar Komponen](#daftar-komponen)
*   [Diagram Pengkabelan](#diagram-pengkabelan)
*   [Persiapan Perangkat Lunak](#persiapan-perangkat-lunak)
*   [Penggunaan](#penggunaan)
*   [Penyelesaian Masalah (Troubleshooting)](#penyelesaian-masalah-troubleshooting)
*   [Kontribusi](#kontribusi)
*   [Lisensi](#lisensi)

## Fitur Utama

*   **Pemantauan Level Air:** Menggunakan sensor ultrasonik untuk mengukur level air di tandon dan mengirimkannya ke Home Assistant.
*   **Pemantauan Suhu Air:** Membaca suhu air menggunakan sensor DS18B20.
*   **Pemantauan Suhu & Kelembaban Udara:** Menggunakan sensor DHT22 untuk memantau kondisi udara sekitar.
*   **Pemantauan TDS:** Mengukur Total Dissolved Solids (konsentrasi nutrisi) di dalam air dengan kompensasi suhu.
*   **Kontrol Pompa Berbasis Volume:** Mengontrol 3 pompa peristaltik (Nutrisi A, Nutrisi B, dan pH) secara presisi berdasarkan volume (ml).
*   **Notifikasi & Peringatan:** Sistem peringatan level air kritis dengan notifikasi MQTT dan buzzer.
*   **Integrasi Home Assistant:** Semua data sensor dan kontrol pompa terintegrasi penuh dengan Home Assistant melalui protokol MQTT.
*   **MQTT Heartbeat:** Mengirim sinyal "hidup" secara berkala ke Home Assistant untuk memverifikasi konektivitas ESP32.
*   **Koneksi Ulang Otomatis:** Koneksi ulang Wi-Fi dan MQTT otomatis jika koneksi terputus.

## Daftar Komponen

| Komponen                      | Jumlah   | Deskripsi                                          |
| :---------------------------- | :------- | :------------------------------------------------- |
| Papan Pengembangan ESP32      | 1        | Direkomendasikan: NodeMCU ESP32, ESP32 DevKitC     |
| Catu Daya DC 12V 5A           | 1        | Untuk memberi daya pada pompa dan modul relay      |
| Sensor Ultrasonik JSN-SR04T   | 1        | Untuk pengukuran level air                         |
| Sensor Suhu DS18B20           | 1        | Untuk suhu air                                     |
| Sensor Suhu & Kelembaban DHT22 | 1        | Untuk suhu & kelembaban udara                      |
| Meteran TDS Analog            | 1        | Untuk mengukur konsentrasi nutrisi air             |
| Buzzer Aktif                  | 1        | Untuk peringatan suara                             |
| Modul Relay 8-Channel         | 1        | Untuk kontrol pompa                                |
| Pompa Peristaltik DC 12V 5W   | 3        | Kamoer NKP-DC-504B atau sejenisnya                 |
| Kabel Jumper                  | Secukupnya | Male-to-Male, Female-to-Female, Male-to-Female     |
| Breadboard (opsional)         | 1        | Untuk prototipe                                    |
| Resistor (opsional)           | Secukupnya | Untuk sensor (misalnya, pull-up DS18B20)           |
| Kotak Enclosure (opsional)    | 1        | Untuk perlindungan & instalasi yang rapi           |

## Diagram Pengkabelan

Berikut adalah koneksi pin detail antara ESP32, Sensor, Buzzer, Catu Daya, Pompa, dan Modul Relay.

**Penting:** Pastikan semua koneksi `GND` terhubung menjadi satu (`Common Ground`). Ini berarti `GND` ESP32, `DC -` modul relay, dan `-V` Catu Daya 12V harus terhubung ke titik yang sama.

### Pinout ESP32

| Pin ESP32 (GPIO) | Terhubung ke Komponen           | Deskripsi                                       |
| :--------------- | :------------------------------ | :---------------------------------------------- |
| 5                | ULTRASONIC_TRIGGER_PIN (JSN-SR04T Trig) | Sinyal Trigger Sensor Ultrasonik                |
| 4                | ULTRASONIC_ECHO_PIN (JSN-SR04T Echo)   | Sinyal Echo Sensor Ultrasonik                   |
| 16               | ONE_WIRE_BUS (DS18B20 Data)            | Pin Data Sensor Suhu Air                        |
| 13               | DHT_PIN (DHT22 Data)                   | Pin Data Sensor Suhu & Kelembaban Udara         |
| 17               | BUZZER_PIN                      | Pin Kontrol Buzzer                              |
| 34               | TDS_SENSOR_PIN                 | Sinyal analog dari papan adapter sensor TDS     |
| 25               | PUMP_NUTRISI_A_PIN (Relay IN1)  | Kontrol Pompa Nutrisi A                         |
| 26               | PUMP_NUTRISI_B_PIN (Relay IN2)  | Kontrol Pompa Nutrisi B                         |
| 27               | PUMP_PH_PIN (Relay IN3)         | Kontrol Pompa pH                                |
| GND              | Semua GND Komponen              | Common Ground Sistem                            |
| 5V (atau VIN)    | Modul Relay DC+                 | Daya untuk Sirkuit Kontrol Relay                |

### Koneksi Catu Daya DC 12V 5A

*   Terminal `L (AC)` dan `N (AC)`: Hubungkan ke sumber listrik AC Anda (misalnya, 220V AC).
*   Terminal `---` (Ground/Arde): (Opsional) Hubungkan ke arde gedung Anda untuk keamanan.
*   Terminal `-V`: Output DC Negatif (GND / Common Ground untuk sistem 12V DC Anda).
*   Terminal `+V`: Output DC Positif (+12V).

### Koneksi Modul Relay 8-Channel

**A. Sisi Kontrol (ESP32 ke Modul Relay):**
*   **Modul Relay `DC -`** --terhubung ke--> **Pin `GND` pada ESP32 Anda**.
    *   (Pastikan ini juga terhubung ke `-V` dari Catu Daya 12V untuk common ground).
*   **Modul Relay `DC +`** --terhubung ke--> **Pin `5V` (atau `VIN`) pada ESP32 Anda**.
*   **Modul Relay `IN1`** --terhubung ke--> **ESP32 GPIO 25 (`PUMP_NUTRISI_A_PIN`)**.
*   **Modul Relay `IN2`** --terhubung ke--> **ESP32 GPIO 26 (`PUMP_NUTRISI_B_PIN`)**.
*   **Modul Relay `IN3`** --terhubung ke--> **ESP32 GPIO 27 (`PUMP_PH_PIN`)**.

**B. Sisi Beban (Catu Daya 12V ke Relay ke Pompa):**

*   **Untuk Setiap Pompa (Nutrisi A, B, pH):**
    *   **Terminal `+V` (+12V)** dari Catu Daya 12V Anda --terhubung ke--> terminal **`COM`** pada channel relay yang Anda gunakan (misalnya, `COM1`, `COM2`, `COM3`).
    *   Terminal **`NO` (Normally Open)** pada channel relay yang Anda gunakan (misalnya, `NO1`, `NO2`, `NO3`) --terhubung ke--> **Kabel Positif (+) Pompa Anda**.
    *   **Kabel Negatif (-) Pompa Anda** --terhubung langsung ke--> **Terminal `-V` (GND) dari Catu Daya 12V Anda**.

**C. Pengaturan Jumper pada Modul Relay:**

*   **Penting:** Pada modul relay Anda, pindahkan jumper `Low - Com - High` untuk menghubungkan pin **`Com` dengan pin `High`** untuk setiap channel yang Anda gunakan (misalnya, channel 1, 2, dan 3). Pengaturan ini membuat relay menjadi "HIGH Level Trigger", yang sesuai dengan kode Anda.

## Persiapan Perangkat Lunak

### Arduino IDE

1.  **Instal Board Manager ESP32:**

    *   Buka Arduino IDE.
    *   Buka `File > Preferences`.
    *   Pada "Additional Boards Manager URLs", tambahkan URL berikut:
        `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
    *   Buka `Tools > Board > Boards Manager...`.
    *   Cari dan instal "esp32 by Espressif Systems".

2.  **Instal Library:**

    *   Buka `Sketch > Include Library > Manage Libraries...`.
    *   Cari dan instal library-library berikut:
        *   `PubSubClient by Nick O'Leary`
        *   `NewPing by Tim Eckel`
        *   `OneWire by Paul Stoffregen`
        *   `DallasTemperature by Miles Burton`
        *   `Adafruit Unified Sensor by Adafruit`
        *   `DHT sensor library by Adafruit`

3.  **Konfigurasi Kode Proyek:**
    *   Buka file `src/config.cpp`. Ini adalah tempat utama untuk semua pengaturan spesifik pengguna.
    *   **Sesuaikan Konfigurasi Wi-Fi:**
        ```cpp
        const char *WIFI_SSID = "SSID_WiFi_Anda";
        const char *WIFI_PASSWORD = "Password_WiFi_Anda";
        ```
    *   **Sesuaikan Konfigurasi MQTT Broker:**
        ```cpp
        const char *MQTT_SERVER = "alamat_broker_mqtt_anda"; // cth., 192.168.1.100
        const int MQTT_PORT = 1883;
        const char *MQTT_USERNAME = "username_mqtt_anda";
        const char *MQTT_PASSWORD = "password_mqtt_anda";
        ```
    *   **Sesuaikan Kalibrasi Sistem & Sensor:** Ubah nilai seperti `MQTT_CLIENT_ID`, `TANDON_MAX_HEIGHT_CM`, `WATER_LEVEL_CRITICAL_CM`, dan `PUMP_MS_PER_ML` sesuai dengan kebutuhan perangkat keras spesifik Anda.
    *   Simpan perubahan.

4.  **Unggah Kode:**

    *   Pilih board ESP32 Anda dari `Tools > Board > ESP32 Arduino`.
    *   Pilih port Serial yang benar dari `Tools > Port`.
    *   Unggah kode ke ESP32 Anda.

### Home Assistant

1.  **Konfigurasi MQTT Broker:**

    *   Pastikan Anda telah menginstal dan mengkonfigurasi MQTT Broker (misalnya, Mosquitto Broker Add-on) di Home Assistant Anda.

2.  **Kontrol Pompa di Home Assistant:**

    *   Karena pompa sekarang dikontrol berdasarkan volume (misalnya, "pompa 50 ml") dan bukan "ON/OFF" sederhana, saklar MQTT standar tidak lagi cocok. Pendekatan yang disarankan adalah menggunakan `input_number` helper dan `script` untuk mengirim volume spesifik.
    *   Tambahkan kode berikut ke file `configuration.yaml` Home Assistant Anda (atau file paket khusus).

    *   **a. Buat `input_number` helper untuk memilih volume:**
        ```yaml
        # configuration.yaml
        input_number:
          pompa_a_volume:
            name: Volume Dosis Nutrisi A
            initial: 10
            min: 1
            max: 200
            step: 1
            unit_of_measurement: "ml"
            icon: mdi:beaker-plus-outline

          pompa_b_volume:
            name: Volume Dosis Nutrisi B
            initial: 10
            min: 1
            max: 200
            step: 1
            unit_of_measurement: "ml"
            icon: mdi:beaker-plus-outline
        ```

    *   **b. Buat `script` untuk memicu pompa dengan volume yang dipilih:**
        ```yaml
        # configuration.yaml
        script:
          dosis_nutrisi_a:
            alias: "Beri Dosis Nutrisi A"
            icon: mdi:water-pump
            sequence:
              - service: mqtt.publish
                data:
                  topic: "hidroponik/pompa/nutrisi_a/kontrol"
                  payload_template: "{{ states('input_number.pompa_a_volume') }}"
          # Buat script serupa untuk pompa_b dan pompa_ph
        ```

3.  **Konfigurasi Sensor MQTT:**

    *   Tambahkan konfigurasi sensor ke file `configuration.yaml` Anda atau file `mqtt_sensors.yaml` yang di-include:
        ```yaml
        # Contoh untuk mqtt_sensors.yaml
        - platform: mqtt
          name: "Level Air Tandon"
          state_topic: "hidroponik/air/level_cm"
          unit_of_measurement: "cm"
          icon: mdi:waves-arrow-up
          value_template: "{{ value | float(0) }}"

        - platform: mqtt
          name: "Jarak Sensor Air"
          state_topic: "hidroponik/air/jarak_sensor_cm"
          unit_of_measurement: "cm"
          icon: mdi:arrow-expand-vertical
          value_template: "{{ value | float(0) }}"

        - platform: mqtt
          name: "Suhu Air"
          state_topic: "hidroponik/air/suhu_c"
          unit_of_measurement: "°C"
          device_class: temperature
          value_template: "{{ value | float(2) }}"

        - platform: mqtt
          name: "Suhu Udara"
          state_topic: "hidroponik/udara/suhu_c"
          unit_of_measurement: "°C"
          device_class: temperature
          value_template: "{{ value | float(2) }}"

        - platform: mqtt
          name: "Kelembaban Udara"
          state_topic: "hidroponik/udara/kelembaban_persen"
          unit_of_measurement: "%"
          device_class: humidity
          value_template: "{{ value | float(2) }}"

        - platform: mqtt
          name: "TDS Air"
          state_topic: "hidroponik/air/tds_ppm"
          unit_of_measurement: "ppm"
          icon: mdi:water-opacity
          value_template: "{{ value | float(2) }}"

        - platform: mqtt
          name: "Status ESP32 Hidroponik"
          state_topic: "tele/esp32hidro/LWT"
          value_template: "{{ value }}"
          icon: mdi:lan-connect

        - platform: mqtt
          name: "Peringatan Hidroponik"
          state_topic: "hidroponik/peringatan"
          value_template: "{{ value }}"
          icon: mdi:alert
        ```

    *   **c. Buat entitas `select` untuk mengontrol mode sistem:**
        ```yaml
        # configuration.yaml
        select:
          - name: "Mode Sistem Hidroponik"
            unique_id: hidroponik_mode_sistem
            state_topic: "hidroponik/sistem/mode/status"
            command_topic: "hidroponik/sistem/mode/kontrol"
            options:
              - "NUTRITION"
              - "CLEANER"
            optimistic: false
            retain: true
        ```

4.  **Restart Home Assistant:** Setelah menambahkan semua konfigurasi, restart Home Assistant agar perubahan diterapkan.

## Penggunaan

Setelah semuanya dirakit dan dikonfigurasi:

1.  Nyalakan Catu Daya DC 12V Anda.
2.  ESP32 akan mencoba terhubung ke Wi-Fi dan kemudian ke Broker MQTT Anda.
3.  Data sensor akan mulai dipublikasikan ke Home Assistant setiap 5 detik.
4.  Anda dapat mengontrol pompa dengan mengirimkan payload numerik (misalnya, `50` untuk 50ml) ke topik `kontrol` pompa. Ini dapat dilakukan melalui skrip Home Assistant (seperti yang dikonfigurasi di atas) atau klien MQTT apa pun.
5.  Untuk melakukan penghentian darurat pada pompa yang sedang berjalan, kirim payload `OFF`.
6.  Jika level air tandon mencapai ambang batas kritis (20 cm atau kurang), sistem akan mengirimkan peringatan MQTT dan mengaktifkan buzzer.

## Penyelesaian Masalah (Troubleshooting)

*   **Pompa tidak aktif setelah mengirim perintah volume:**
    *   **Periksa Koneksi Kabel:** Pastikan semua kabel terhubung dengan aman dan benar, terutama `GND`, yang harus menjadi ground bersama untuk semua komponen (ESP32, Relay, Catu Daya).
    *   **Periksa Daya Relay:** Pastikan modul relay menerima daya 5V yang cukup dan stabil pada pin `DC+`-nya.
    *   **Periksa Pengaturan Jumper Relay:** Pastikan jumper `Low - Com - High` pada modul relay diatur ke posisi **`Com - High`**. Ini memastikan relay berfungsi sebagai "HIGH Level Trigger" sesuai dengan kode Anda.
    *   **Periksa Serial Monitor:** Saat Anda mengirim perintah, monitor serial ESP32 seharusnya mencetak pesan `[Pump Control] Pumping X ml...`. Jika tidak, berarti pesan MQTT tidak diterima atau tidak di-parse dengan benar.
*   **Sensor tidak membaca data:**
    *   Periksa kembali koneksi pin data sensor ke ESP32.
    *   Pastikan library sensor telah diinstal dengan benar.
*   **ESP32 tidak terhubung ke Wi-Fi/MQTT:**
    *   Periksa kembali kredensial Anda di `src/config.cpp`.
    *   Pastikan ESP32 berada dalam jangkauan Wi-Fi dan Broker MQTT Anda dapat diakses.
    *   Gunakan Serial Monitor Arduino IDE untuk melihat log koneksi.

## Kontribusi

Kontribusi untuk proyek ini sangat diterima! Jika Anda memiliki saran, perbaikan, atau menemukan bug, jangan ragu untuk membuka issue atau mengirimkan pull request.

## Lisensi

Proyek ini dilisensikan di bawah Lisensi MIT. Anda bebas menggunakan, memodifikasi, dan mendistribusikan kode ini untuk tujuan pribadi atau komersial, dengan atribusi yang sesuai.