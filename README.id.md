# Proyek Sistem Otomatisasi Hidroponik ESP32

Proyek ini adalah sistem otomatisasi untuk budidaya hidroponik menggunakan ESP32, MQTT, dan terintegrasi dengan Home Assistant. Sistem ini memantau parameter penting seperti level air, suhu air, suhu udara, kelembaban, dan Total Dissolved Solids (TDS). Selain itu, sistem ini menyediakan kontrol presisi berbasis volume untuk pompa nutrisi dan pH, serta sistem peringatan dini untuk level air kritis.

## Daftar Isi

- [Proyek Sistem Otomatisasi Hidroponik ESP32](#proyek-sistem-otomatisasi-hidroponik-esp32)
  - [Daftar Isi](#daftar-isi)
  - [Fitur Utama](#fitur-utama)
  - [Dukungan Multi-Instance](#dukungan-multi-instance)
  - [Daftar Komponen](#daftar-komponen)
  - [Arsitektur Sistem (Pengaturan Dua Box)](#arsitektur-sistem-pengaturan-dua-box)
    - [Koneksi Antar-Box (Kabel LAN)](#koneksi-antar-box-kabel-lan)
  - [Diagram Pengkabelan](#diagram-pengkabelan)
    - [Pinout untuk Instance `Produksi`](#pinout-untuk-instance-produksi)
    - [Pinout untuk Instance `Penyemaian`](#pinout-untuk-instance-penyemaian)
    - [Koneksi Catu Daya DC 12V 5A](#koneksi-catu-daya-dc-12v-5a)
    - [Koneksi Modul Relay 8-Channel](#koneksi-modul-relay-8-channel)
  - [Persiapan Perangkat Lunak (PlatformIO)](#persiapan-perangkat-lunak-platformio)
  - [Konfigurasi Home Assistant](#konfigurasi-home-assistant)
  - [Penggunaan](#penggunaan)
  - [Penyelesaian Masalah (Troubleshooting)](#penyelesaian-masalah-troubleshooting)
  - [Kontribusi](#kontribusi)
  - [Lisensi](#lisensi)

## Fitur Utama

*   **Dukungan Multi-Instance:** Satu basis kode untuk menghasilkan firmware yang berbeda untuk dua jenis instalasi (`produksi` dan `penyemaian`).
*   **Pemantauan Level Air:** Menggunakan sensor ultrasonik untuk mengukur level air di tandon dan mengirimkannya ke Home Assistant.
*   **Pemantauan Suhu Air:** Membaca suhu air menggunakan sensor DS18B20.
*   **Pemantauan Suhu & Kelembaban Udara:** Menggunakan sensor DHT22 untuk memantau kondisi udara sekitar.
*   **Pemantauan TDS:** Mengukur Total Dissolved Solids (konsentrasi nutrisi) di dalam air dengan kompensasi suhu.
*   **Pemantauan pH:** Mengukur tingkat keasaman (pH) air menggunakan sensor analog PH-4502C.
*   **Pemantauan Konsumsi Listrik:** Mengukur tegangan, arus, daya, dan energi menggunakan sensor PZEM-004T.
*   **Kontrol Pompa Berbasis Volume:** Mengontrol 3 pompa peristaltik (Nutrisi A, Nutrisi B, dan pH) secara presisi berdasarkan volume (ml).
*   **Notifikasi & Peringatan:** Sistem peringatan level air kritis dengan notifikasi MQTT dan buzzer.
*   **Integrasi Home Assistant:** Semua data sensor dan kontrol pompa terintegrasi penuh dengan Home Assistant melalui protokol MQTT.
*   **MQTT Heartbeat:** Mengirim sinyal "hidup" secara berkala ke Home Assistant untuk memverifikasi konektivitas ESP32.
*   **Koneksi Ulang Otomatis:** Koneksi ulang Wi-Fi dan MQTT otomatis jika koneksi terputus.
*   **Manajemen Kredensial Aman:** Memisahkan kredensial Wi-Fi dan MQTT dari kode utama menggunakan file `credentials.ini`.

## Dukungan Multi-Instance

Proyek ini dirancang untuk mengelola dua jenis sistem hidroponik dari satu basis kode yang sama:
1.  **Instance `produksi`:** Sistem dengan fitur lengkap, termasuk semua sensor, 3 pompa dosis, mode sistem, dan peringatan.
2.  **Instance `penyemaian`:** Sistem sederhana yang hanya memiliki satu fungsi: mengontrol pompa penyiraman berdasarkan durasi.

## Daftar Komponen

| Komponen                      | Jumlah (Produksi) | Jumlah (Penyemaian) | Deskripsi                                          |
| :---------------------------- | :---------------- | :------------------ | :------------------------------------------------- |
| Papan Pengembangan ESP32      | 1                 | 1                   | Direkomendasikan: NodeMCU ESP32, ESP32 DevKitC     |
| Catu Daya DC 12V              | 1                 | 1                   | Untuk memberi daya pada pompa dan modul relay      |
| Pompa DC 12V                  | 3 (Peristaltik)   | 1 (Bebas)           | Kamoer NKP (produksi) atau pompa air biasa (penyemaian) |
| Modul Relay                   | 1 (min. 3 channel)| 1 (min. 1 channel)  | Untuk kontrol pompa                                |
| Sensor Ultrasonik JSN-SR04T   | 1                 | 0                   | Untuk pengukuran level air                         |
| Sensor Suhu DS18B20           | 1                 | 0                   | Untuk suhu air                                     |
| Sensor Suhu & Kelembaban DHT22 | 1                 | 0                   | Untuk suhu & kelembaban udara                      |
| Meteran TDS Analog            | 1                 | 0                   | Untuk mengukur konsentrasi nutrisi air             |
| Sensor pH PH-4502C            | 1                 | 0                   | Untuk mengukur tingkat pH air                      |
| Sensor Listrik PZEM-004T v4   | 1                 | 0                   | Untuk mengukur konsumsi listrik                    |
| Buzzer Aktif                  | 1                 | 0                   | Untuk peringatan suara                             |
| Kabel Jumper                  | Secukupnya        | Secukupnya          | Male-to-Male, Female-to-Female, Male-to-Female     |
| Kotak Enclosure (opsional)    | 1                 | 1                   | Untuk perlindungan & instalasi yang rapi           |

## Arsitektur Sistem (Pengaturan Dua Box)

Untuk meningkatkan keamanan dan modularitas, sistem ini dirancang untuk dipisah menjadi dua box (kotak) terpisah:

*   **Box 1 (Box Kontroler):** Berisi komponen pemrosesan utama dan komponen tegangan tinggi, ditempatkan di area yang kering.
    *   Papan Pengembangan ESP32
    *   Sensor Listrik PZEM-004T
    *   Modul Relay
    *   Buzzer
    *   Sensor Suhu & Kelembaban Udara DHT22

*   **Box 2 (Box Sensor):** Berisi semua sensor "basah" yang ditempatkan di dekat atau di dalam tandon air.
    *   Sensor Ultrasonik JSN-SR04T
    *   Sensor Suhu Air DS18B20
    *   Sensor TDS Analog
    *   Sensor pH PH-4502C

### Koneksi Antar-Box (Kabel LAN)

Satu buah **kabel LAN Cat5e atau Cat6** (dengan panjang hingga 2-3 meter) digunakan untuk menghubungkan kedua box. Ini menyediakan cara yang rapi dan terorganisir untuk menyalurkan semua kabel yang diperlukan.

| Fungsi Sinyal          | Pin ESP32 (di Box 1) | Warna Kabel LAN (Standar T568B) | Keterangan                               |
| :--------------------- | :------------------- | :------------------------------ | :--------------------------------------- |
| **Daya 5V**            | `5V`                 | Coklat                          | Daya untuk Sensor Ultrasonik             |
| **Ground**             | `GND`                | Putih-Coklat                    | **Common Ground** untuk semua sensor     |
| **Daya 3.3V**          | `3V3`                | Biru                            | Daya untuk sensor pH, TDS, dan DS18B20   |
| **Data Suhu Air**      | `GPIO 18`            | Putih-Biru                      | Sinyal data dari DS18B20 (One-Wire)      |
| **Ultrasonic Trigger** | `GPIO 5`             | Oranye                          | Sinyal *trigger* ke sensor ultrasonik    |
| **Ultrasonic Echo**    | `GPIO 4`             | Putih-Oranye                    | Sinyal *echo* dari sensor ultrasonik     |
| **Data pH**            | `GPIO 32`            | Hijau                           | Sinyal analog dari sensor pH             |
| **Data TDS**           | `GPIO 34`            | Putih-Hijau                     | Sinyal analog dari sensor TDS            |

**⚠️ Peringatan Penting:** Memperpanjang kabel sensor, terutama untuk sinyal analog (pH, TDS) dan sinyal digital yang sensitif terhadap waktu (Ultrasonik, DS18B20), sejauh 2-3 meter dapat menimbulkan gangguan (noise) dan degradasi sinyal. Hal ini dapat menyebabkan pembacaan yang tidak akurat atau tidak stabil. Meskipun pengaturan ini dapat berfungsi, solusi profesional yang lebih andal adalah dengan menempatkan mikrokontroler kedua (seperti ESP8266) di Box 2 untuk memproses data sensor secara lokal dan mengirimkannya secara digital. Lanjutkan dengan pengaturan satu kontroler ini atas pertimbangan Anda sendiri.

## Diagram Pengkabelan

Berikut adalah koneksi pin detail antara ESP32, Sensor, Buzzer, Catu Daya, Pompa, dan Modul Relay.

**Penting:** Pastikan semua koneksi `GND` terhubung menjadi satu (`Common Ground`). Ini berarti `GND` ESP32, `DC -` modul relay, dan `-V` Catu Daya 12V harus terhubung ke titik yang sama.

### Pinout untuk Instance `Produksi`

| Pin ESP32 (GPIO) | Terhubung ke Komponen           | Deskripsi                                       |
| :--------------- | :------------------------------ | :---------------------------------------------- |
| 5                | ULTRASONIC_TRIGGER_PIN (JSN-SR04T Trig) | Sinyal Trigger Sensor Ultrasonik                |
| 4                | ULTRASONIC_ECHO_PIN (JSN-SR04T Echo)   | Sinyal Echo Sensor Ultrasonik                   |
| 18               | ONE_WIRE_BUS (DS18B20 Data)            | Pin Data Sensor Suhu Air                        |
| 13               | DHT_PIN (DHT22 Data)                   | Pin Data Sensor Suhu & Kelembaban Udara         |
| 19               | BUZZER_PIN                      | Pin Kontrol Buzzer                              |
| 34               | TDS_SENSOR_PIN                 | Sinyal analog dari papan adapter sensor TDS     |
| 32               | PH_SENSOR_PIN                  | Sinyal analog dari papan adapter sensor pH      |
| 16               | PZEM-004T TX (ke ESP32 RX2)     | Data Keluar Sensor Listrik                      |
| 17               | PZEM-004T RX (ke ESP32 TX2)     | Data Masuk Sensor Listrik                       |
| 25               | PUMP_NUTRISI_A_PIN (Relay IN1)  | Kontrol Pompa Nutrisi A                         |
| 26               | PUMP_NUTRISI_B_PIN (Relay IN2)  | Kontrol Pompa Nutrisi B                         |
| 27               | PUMP_PH_PIN (Relay IN3)         | Kontrol Pompa pH                                |
| GND              | Semua GND Komponen              | Common Ground Sistem                            |
| 5V (atau VIN)    | Modul Relay DC+ & PZEM-004T     | Daya untuk Sirkuit Kontrol                      |
| 3V3              | Modul pH & TDS VCC              | Daya untuk Modul Sensor Analog                  |

### Pinout untuk Instance `Penyemaian`

| Pin ESP32 (GPIO) | Terhubung ke Komponen           | Deskripsi                                       |
| :--------------- | :------------------------------ | :---------------------------------------------- |
| 32               | PUMP_SIRAM_PIN (Relay INx)      | Kontrol Pompa Penyiraman                        |
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
*   **Modul Relay `DC +`** --terhubung ke--> **Pin `5V` (atau `VIN`) pada ESP32 Anda**.
*   **Modul Relay `IN1`** --terhubung ke--> **ESP32 GPIO 25 (`PUMP_NUTRISI_A_PIN`)**.
*   **Modul Relay `IN2`** --terhubung ke--> **ESP32 GPIO 26 (`PUMP_NUTRISI_B_PIN`)**.
*   **Modul Relay `IN3`** --terhubung ke--> **ESP32 GPIO 27 (`PUMP_PH_PIN`)**.
*   **Modul Relay `INx`** --terhubung ke--> **ESP32 GPIO 32 (`PUMP_SIRAM_PIN`)** (untuk instance penyemaian).

**B. Sisi Beban (Catu Daya 12V ke Relay ke Pompa):**

*   **Untuk Setiap Pompa (Nutrisi A, B, pH):**
    *   **Terminal `+V` (+12V)** dari Catu Daya 12V Anda --terhubung ke--> terminal **`COM`** pada channel relay yang Anda gunakan (misalnya, `COM1`, `COM2`, `COM3`).
    *   Terminal **`NO` (Normally Open)** pada channel relay yang Anda gunakan (misalnya, `NO1`, `NO2`, `NO3`) --terhubung ke--> **Kabel Positif (+) Pompa Anda**.
    *   **Kabel Negatif (-) Pompa Anda** --terhubung langsung ke--> **Terminal `-V` (GND) dari Catu Daya 12V Anda**.

**C. Pengaturan Jumper pada Modul Relay:**
*   **Penting:** Pada modul relay Anda, pindahkan jumper `Low - Com - High` untuk menghubungkan pin **`Com` dengan pin `High`** untuk setiap channel yang Anda gunakan (misalnya, channel 1, 2, dan 3). Pengaturan ini membuat relay menjadi "HIGH Level Trigger", yang sesuai dengan kode Anda.

## Persiapan Perangkat Lunak (PlatformIO)

Proyek ini dirancang untuk dikompilasi menggunakan **PlatformIO** di dalam Visual Studio Code.

1.  **Instal VS Code & PlatformIO:**
    *   Unduh dan instal Visual Studio Code.
    *   Buka VS Code, pergi ke tab Extensions (ikon kotak), cari dan instal ekstensi **PlatformIO IDE**.

2.  **Buka Proyek:**
    *   Clone repositori ini atau unduh sebagai ZIP dan ekstrak.
    *   Di VS Code, buka `File > Open Folder...` dan pilih direktori proyek `HidroIot`. PlatformIO akan secara otomatis mendeteksi `platformio.ini` dan menginstal semua dependensi yang diperlukan.

3.  **Buat File `credentials.ini`:**
    *   Di direktori utama proyek (di level yang sama dengan `platformio.ini`), buat file baru bernama `credentials.ini`.
    *   Isi file tersebut dengan kredensial Anda. File ini **tidak akan** diunggah ke Git.
        ```ini
        [credentials]
        wifi_ssid = "SSID_WiFi_Anda"
        wifi_password = "Password_WiFi_Anda"
        mqtt_user = "Username_MQTT_Anda"
        mqtt_pass = "Password_MQTT_Anda"
        ```

4.  **Pilih Environment dan Unggah:**
    *   Hubungkan papan ESP32 Anda ke komputer.
    *   Di bagian bawah jendela VS Code, Anda akan melihat status bar PlatformIO. Klik pada nama environment (misalnya, `Default (produksi)`).
    *   Pilih environment yang ingin Anda unggah: **`produksi`** atau **`penyemaian`**.
    *   Klik tombol **Upload** (ikon panah ke kanan) di status bar. PlatformIO akan mengompilasi dan mengunggah firmware yang benar ke perangkat Anda.

## Konfigurasi Home Assistant

1.  **Konfigurasi MQTT Broker:**
    *   Pastikan Anda telah menginstal dan mengkonfigurasi MQTT Broker (misalnya, Mosquitto Broker Add-on) di Home Assistant Anda.

2.  **Konfigurasi Entitas di Home Assistant:**
    *   Karena proyek ini sekarang mendukung beberapa *instance*, Anda perlu menambahkan konfigurasi YAML yang sesuai untuk setiap ESP32 yang Anda pasang.
    *   Salin blok kode di bawah ini ke file-file yang sesuai (`configuration.yaml`, `input_numbers.yaml`, `scripts.yaml`).

    ---

    ### **Konfigurasi untuk Instance `PRODUKSI`**

    *   **a. Tambahkan ke `input_numbers.yaml`:**
        ```yaml
        # Helper untuk memilih volume pompa (Instance Produksi)
        produksi_pompa_a_volume:
          name: "Produksi - Volume Dosis A"
          initial: 20
          min: 1
          max: 200
          step: 1
          unit_of_measurement: "ml"
          icon: mdi:beaker-plus-outline
          mode: box

        produksi_pompa_b_volume:
          name: "Produksi - Volume Dosis B"
          initial: 20
          min: 1
          max: 200
          step: 1
          unit_of_measurement: "ml"
          icon: mdi:beaker-plus-outline
          mode: box

        produksi_pompa_ph_volume:
          name: "Produksi - Volume Dosis pH"
          initial: 10
          min: 1
          max: 100
          step: 1
          unit_of_measurement: "ml"
          icon: mdi:ph
          mode: box
        ```

    *   **b. Tambahkan ke `scripts.yaml`:**
        ```yaml
        # =================================================
        # == SCRIPT UNTUK INSTANCE 'PRODUKSI'
        # =================================================
        produksi_dosis_nutrisi_a:
          alias: "Produksi - Dosis Nutrisi A"
          icon: mdi:water-pump
          sequence:
            - service: mqtt.publish
              data:
                topic: "hidroponik/produksi/pompa/nutrisi_a/kontrol"
                payload: "{{ states('input_number.produksi_pompa_a_volume') | int(0) }}"
                retain: false

        produksi_dosis_nutrisi_b:
          alias: "Produksi - Dosis Nutrisi B"
          icon: mdi:water-pump
          sequence:
            - service: mqtt.publish
              data:
                topic: "hidroponik/produksi/pompa/nutrisi_b/kontrol"
                payload: "{{ states('input_number.produksi_pompa_b_volume') | int(0) }}"
                retain: false

        produksi_dosis_ph:
          alias: "Produksi - Dosis pH"
          icon: mdi:water-pump
          sequence:
            - service: mqtt.publish
              data:
                topic: "hidroponik/produksi/pompa/ph/kontrol"
                payload: "{{ states('input_number.produksi_pompa_ph_volume') | int(0) }}"
                retain: false
        ```

    *   **c. Tambahkan ke `configuration.yaml` (di bawah `mqtt:`):**
        ```yaml
        # Konfigurasi MQTT (Format Modern & Lengkap)
        mqtt:
          # =================================================
          # == ENTITAS UNTUK INSTANCE 'PRODUKSI'
          # =================================================
          sensor:
            - name: "Produksi - Level Air Tandon"
              unique_id: hidroponik_produksi_level_air
              state_topic: "hidroponik/produksi/air/level_cm"
              unit_of_measurement: "cm"
              icon: mdi:waves-arrow-up
              value_template: "{{ value | float(0) }}"
              availability_topic: "hidroponik/produksi/status/LWT"
              payload_available: "Online"
              payload_not_available: "Offline"

            - name: "Produksi - Suhu Air"
              unique_id: hidroponik_produksi_suhu_air
              state_topic: "hidroponik/produksi/air/suhu_c"
              unit_of_measurement: "°C"
              device_class: temperature
              value_template: "{{ value | float(2) }}"
              availability_topic: "hidroponik/produksi/status/LWT"
              payload_available: "Online"
              payload_not_available: "Offline"

            - name: "Produksi - Suhu Udara"
              unique_id: hidroponik_produksi_suhu_udara
              state_topic: "hidroponik/produksi/udara/suhu_c"
              unit_of_measurement: "°C"
              device_class: temperature
              value_template: "{{ value | float(2) }}"
              availability_topic: "hidroponik/produksi/status/LWT"
              payload_available: "Online"
              payload_not_available: "Offline"

            - name: "Produksi - Kelembaban Udara"
              unique_id: hidroponik_produksi_kelembaban_udara
              state_topic: "hidroponik/produksi/udara/kelembaban_persen"
              unit_of_measurement: "%"
              device_class: humidity
              value_template: "{{ value | float(2) }}"
              availability_topic: "hidroponik/produksi/status/LWT"
              payload_available: "Online"
              payload_not_available: "Offline"

            - name: "Produksi - TDS Air"
              unique_id: hidroponik_produksi_tds_air
              state_topic: "hidroponik/produksi/air/tds_ppm"
              unit_of_measurement: "ppm"
              icon: mdi:water-opacity
              value_template: "{{ value | float(2) }}"
              availability_topic: "hidroponik/produksi/status/LWT"
              payload_available: "Online"
              payload_not_available: "Offline"

          binary_sensor:
            - name: "Produksi - Status ESP32"
              unique_id: hidroponik_produksi_status_online
              state_topic: "hidroponik/produksi/status/LWT"
              payload_on: "Online"
              payload_off: "Offline"
              device_class: connectivity

          # Kontrol untuk Mode Sistem (hanya untuk Produksi)
          select:
            - name: "Produksi - Mode Sistem"
              unique_id: hidroponik_produksi_mode_sistem
              state_topic: "hidroponik/produksi/sistem/mode/status"
              command_topic: "hidroponik/produksi/sistem/mode/kontrol"
              options:
                - "NUTRITION"
                - "CLEANER"
              optimistic: false
              retain: true
              availability_topic: "hidroponik/produksi/status/LWT"
              payload_available: "Online"
              payload_not_available: "Offline"
        ```

    ---

    ### **Konfigurasi untuk Instance `PENYEMAIAN`**

    *   **a. Tambahkan ke `input_numbers.yaml`:**
        ```yaml
        # Helper untuk durasi penyiraman (Instance Penyemaian)
        penyemaian_pompa_siram_durasi:
          name: "Penyemaian - Durasi Siram"
          initial: 15
          min: 1
          max: 300 # Batas atas 5 menit, bisa disesuaikan
          step: 1
          unit_of_measurement: "detik"
          icon: mdi:timer-sand
          mode: box
        ```

    *   **b. Tambahkan ke `scripts.yaml`:**
        ```yaml
        # =================================================
        # == SCRIPT UNTUK INSTANCE 'PENYEMAIAN'
        # =================================================
        siram_penyemaian:
          alias: "Penyemaian - Siram Pompa"
          icon: mdi:sprinkler-variant
          sequence:
            - service: mqtt.publish
              data:
                topic: "hidroponik/penyemaian/pompa/siram/kontrol"
                payload: "{{ states('input_number.penyemaian_pompa_siram_durasi') | int(0) }}"
                retain: false
        ```

    *   **c. Tambahkan ke `configuration.yaml` (di bawah `mqtt: > binary_sensor:`):**
        ```yaml
          binary_sensor:
            # ... (sensor biner produksi di atas baris ini) ...
            # =================================================
            # == ENTITAS UNTUK INSTANCE 'PENYEMAIAN'
            # =================================================
            - name: "Penyemaian - Status ESP32"
              unique_id: hidroponik_penyemaian_status_online
              state_topic: "hidroponik/penyemaian/status/LWT"
              payload_on: "Online"
              payload_off: "Offline"
              device_class: connectivity
        ```
## Penggunaan

Setelah semua dirakit dan dikonfigurasi:

1.  Nyalakan Catu Daya DC 12V Anda.
2.  ESP32 akan mencoba terhubung ke Wi-Fi dan kemudian ke Broker MQTT Anda.
3.  Data sensor (untuk instance `produksi`) akan mulai dipublikasikan ke Home Assistant.
4.  Anda dapat mengontrol pompa melalui dasbor Home Assistant.
5.  Untuk melakukan penghentian darurat pada pompa yang sedang berjalan, kirim payload `OFF`.
6.  Jika level air tandon (untuk instance `produksi`) mencapai ambang batas kritis, sistem akan mengirimkan peringatan MQTT dan mengaktifkan buzzer.

## Penyelesaian Masalah (Troubleshooting)

*   **Pompa tidak aktif setelah mengirim perintah volume:**
    *   **Periksa Koneksi Kabel:** Pastikan semua kabel terhubung dengan aman dan benar, terutama `GND`, yang harus menjadi ground bersama untuk semua komponen (ESP32, Relay, Catu Daya).
    *   **Periksa Daya Relay:** Pastikan modul relay menerima daya 5V yang cukup dan stabil pada pin `DC+`-nya.
    *   **Periksa Pengaturan Jumper Relay:** Pastikan jumper `Low - Com - High` pada modul relay diatur ke posisi **`Com - High`**. Ini memastikan relay berfungsi sebagai "HIGH Level Trigger" sesuai dengan kode Anda.
    *   **Periksa Serial Monitor:** Saat Anda mengirim perintah, monitor serial ESP32 seharusnya mencetak pesan `[Pump Control] Pumping X ml...`. Jika tidak, berarti pesan MQTT tidak diterima atau tidak di-parse dengan benar.
*   **Sensor tidak membaca data (Instance Produksi):**
    *   Periksa kembali koneksi pin data sensor ke ESP32.
    *   Pastikan library sensor yang relevan telah terinstal dengan benar oleh PlatformIO.
*   **ESP32 tidak terhubung ke Wi-Fi/MQTT:**
    *   Periksa kembali kredensial Anda di file `credentials.ini`.
    *   Pastikan ESP32 berada dalam jangkauan Wi-Fi dan Broker MQTT Anda dapat diakses.
    *   Gunakan Serial Monitor di PlatformIO untuk melihat log koneksi secara detail.

## Kontribusi

Kontribusi untuk proyek ini sangat diterima! Jika Anda memiliki saran, perbaikan, atau menemukan bug, jangan ragu untuk membuka *issue* atau mengirimkan *pull request*.

## Lisensi

Proyek ini dilisensikan di bawah Lisensi MIT. Anda bebas menggunakan, memodifikasi, dan mendistribusikan kode ini untuk keperluan pribadi atau komersial, dengan atribusi yang sesuai.
