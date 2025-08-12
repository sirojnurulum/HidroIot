# Proyek Sistem Otomatisasi Hidroponik ESP32

Proyek ini adalah sistem otomatisasi untuk budidaya hidroponik menggunakan ESP32, MQTT, dan terintegrasi dengan Home Assistant. Sistem ini memantau parameter penting seperti level air, suhu air, suhu udara, kelembaban, dan Total Dissolved Solids (TDS). Selain itu, sistem ini menyediakan kontrol presisi berbasis volume untuk pompa nutrisi dan pH, serta sistem peringatan dini untuk level air kritis.

## Daftar Isi

- [Proyek Sistem Otomatisasi Hidroponik ESP32](#proyek-sistem-otomatisasi-hidroponik-esp32)
  - [Daftar Isi](#daftar-isi)
  - [Fitur Utama](#fitur-utama)
  - [Daftar Komponen](#daftar-komponen)
  - [Arsitektur Sistem (Pengaturan Dua Box)](#arsitektur-sistem-pengaturan-dua-box)
    - [Koneksi Antar-Box (Kabel LAN)](#koneksi-antar-box-kabel-lan)
  - [Diagram Pengkabelan](#diagram-pengkabelan)
    - [Pinout ESP32](#pinout-esp32)
    - [Koneksi Catu Daya DC 12V 5A](#koneksi-catu-daya-dc-12v-5a)
    - [Koneksi Modul Relay 8-Channel](#koneksi-modul-relay-8-channel)
  - [Persiapan Perangkat Lunak (PlatformIO)](#persiapan-perangkat-lunak-platformio)
  - [Konfigurasi Home Assistant](#konfigurasi-home-assistant)
  - [Penggunaan](#penggunaan)
  - [Penyelesaian Masalah (Troubleshooting)](#penyelesaian-masalah-troubleshooting)
  - [Kontribusi](#kontribusi)
  - [Lisensi](#lisensi)

## Fitur Utama

*   **Pemantauan Komprehensif:**
    *   **Air:** Level (Ultrasonik), Suhu (DS18B20), TDS (dengan kompensasi suhu), dan pH (dengan kalibrasi 4-titik).
    *   **Lingkungan:** Suhu & Kelembaban Udara (DHT22).
    *   **Kelistrikan:** Tegangan, Arus, Daya, Energi, Frekuensi, dan Power Factor (PZEM-004T).
*   **Kontrol Manual Presisi:**
    *   **Dosis:** Kontrol pompa Nutrisi A, B, dan pH berdasarkan volume (ml).
    *   **Penyiraman:** Kontrol pompa penyiraman berdasarkan durasi (detik).
    *   **Pengisian Tandon:** Kontrol manual ON/OFF untuk katup pengisian.
*   **Suite Automasi Penuh:**
    *   **Auto-Dosing:** Menjaga level TDS dan pH secara otomatis berdasarkan target yang dapat diatur.
    *   **Auto-Refill:** Mengisi ulang tandon secara otomatis saat level air rendah dan berhenti saat penuh.
    *   **Penyiraman Cerdas:** Menjalankan penyiraman terjadwal (setiap jam) dan berdasarkan suhu udara yang tinggi.
*   **Integrasi Home Assistant Tingkat Lanjut:**
    *   Semua data sensor dan kontrol aktuator terintegrasi penuh melalui MQTT.
    *   Dasbor kustom dengan 3 tab: Pemantauan, Kontrol Manual, dan Pengaturan.
    *   Sinkronisasi dua arah antara status automasi di UI dan di perangkat ESP32.
*   **Keandalan & Keamanan:**
    *   Koneksi ulang Wi-Fi dan MQTT otomatis.
    *   MQTT Last Will and Testament (LWT) untuk status online/offline yang akurat.
    *   Peringatan level air kritis dengan buzzer dan notifikasi MQTT.
    *   Manajemen kredensial aman menggunakan file `credentials.ini` yang terpisah.

## Daftar Komponen

| Komponen                      | Jumlah | Deskripsi                                          |
| :---------------------------- | :----: | :------------------------------------------------- |
| Papan Pengembangan ESP32      | 1      | Direkomendasikan: NodeMCU ESP32, ESP32 DevKitC     |
| Catu Daya DC 12V              | 1      | Untuk memberi daya pada pompa dan modul relay      |
| Pompa DC 12V                  | 4      | 3x Peristaltik (Nutrisi/pH), 1x Pompa Siram        |
| Modul Relay                   | 1      | Direkomendasikan: 4-channel atau 8-channel       |
| Sensor Ultrasonik JSN-SR04T   | 1      | Untuk pengukuran level air                         |
| Sensor Suhu DS18B20           | 1      | Untuk suhu air                                     |
| Sensor Suhu & Kelembaban DHT22 | 1      | Untuk suhu & kelembaban udara                      |
| Meteran TDS Analog            | 1      | Untuk mengukur konsentrasi nutrisi air             |
| Sensor pH PH-4502C            | 1      | Untuk mengukur tingkat pH air                      |
| Sensor Listrik PZEM-004T v4   | 1      | Untuk mengukur konsumsi listrik                    |
| Buzzer Aktif                  | 1      | Untuk peringatan suara                             |
| Kabel Jumper                  | -      | Male-to-Male, Female-to-Female, Male-to-Female     |
| Kotak Enclosure (opsional)    | 2      | Satu untuk kontroler, satu untuk sensor basah      |

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

### Pinout ESP32

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
| 33               | PUMP_SIRAM_PIN (Relay IN4)      | Kontrol Pompa Penyiraman                        |
| 15               | PUMP_TANDON_PIN (Relay IN5)     | Kontrol Katup/Pompa Pengisian Tandon            |
| GND              | Semua GND Komponen              | Common Ground Sistem                            |
| 5V (atau VIN)    | Modul Relay DC+ & PZEM-004T     | Daya untuk Sirkuit Kontrol                      |
| 3V3              | Modul pH & TDS VCC              | Daya untuk Modul Sensor Analog                  |

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
*   **Modul Relay `IN4`** --terhubung ke--> **ESP32 GPIO 33 (`PUMP_SIRAM_PIN`)**.
*   **Modul Relay `IN5`** --terhubung ke--> **ESP32 GPIO 15 (`PUMP_TANDON_PIN`)**.

**B. Sisi Beban (Catu Daya 12V ke Relay ke Pompa):**

*   **Untuk Setiap Pompa:**
    *   **Terminal `+V` (+12V)** dari Catu Daya 12V Anda --terhubung ke--> terminal **`COM`** pada channel relay yang Anda gunakan (misalnya, `COM1`, `COM2`, `COM3`).
    *   Terminal **`NO` (Normally Open)** pada channel relay yang Anda gunakan (misalnya, `NO1`, `NO2`, `NO3`) --terhubung ke--> **Kabel Positif (+) Pompa Anda**.
    *   **Kabel Negatif (-) Pompa Anda** --terhubung langsung ke--> **Terminal `-V` (GND) dari Catu Daya 12V Anda**.

**C. Pengaturan Jumper pada Modul Relay:**
*   **Penting:** Pada modul relay Anda, pindahkan jumper `Low - Com - High` untuk menghubungkan pin **`Com` dengan pin `High`** untuk setiap channel yang Anda gunakan. Pengaturan ini membuat relay menjadi "HIGH Level Trigger", yang sesuai dengan kode Anda.

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
        wifi_ssid = "Your_WiFi_SSID"
        wifi_password = "Your_WiFi_Password"
        mqtt_user = "Your_MQTT_Username"
        mqtt_pass = "Your_MQTT_Password"
        mqtt_server = "your_mqtt_broker_ip_or_domain"
        mqtt_port = 1883
        ```

4.  **Unggah Firmware:**
    *   Hubungkan papan ESP32 Anda ke komputer.
    *   Klik tombol **Upload** (ikon panah ke kanan) di status bar PlatformIO di bagian bawah jendela VS Code. PlatformIO akan mengompilasi dan mengunggah firmware ke perangkat Anda.

## Konfigurasi Home Assistant

1.  **Konfigurasi MQTT Broker:**
    *   Pastikan Anda telah menginstal dan mengkonfigurasi MQTT Broker (misalnya, Mosquitto Broker Add-on) di Home Assistant Anda.

2.  **Konfigurasi Entitas di Home Assistant:**
    *   Salin semua file dan direktori dari `src/ha_config/` proyek ini ke dalam direktori konfigurasi Home Assistant Anda. Pastikan struktur file dipertahankan.
    *   Atau, gunakan `Makefile` yang disediakan untuk men-deploy konfigurasi secara otomatis.

## Penggunaan

Setelah semua dirakit dan dikonfigurasi:

1.  Nyalakan Catu Daya DC 12V Anda.
2.  ESP32 akan mencoba terhubung ke Wi-Fi dan kemudian ke Broker MQTT Anda.
3.  Data sensor akan mulai dipublikasikan ke Home Assistant.
4.  Anda dapat mengontrol pompa melalui dasbor Home Assistant.
5.  Untuk melakukan penghentian darurat pada pompa yang sedang berjalan, kirim payload `OFF`.
6.  Jika level air tandon mencapai ambang batas kritis, sistem akan mengirimkan peringatan MQTT dan mengaktifkan buzzer.

## Penyelesaian Masalah (Troubleshooting)

*   **Pompa tidak aktif setelah mengirim perintah volume:**
    *   **Periksa Koneksi Kabel:** Pastikan semua kabel terhubung dengan aman dan benar, terutama `GND`, yang harus menjadi ground bersama untuk semua komponen (ESP32, Relay, Catu Daya).
    *   **Periksa Daya Relay:** Pastikan modul relay menerima daya 5V yang cukup dan stabil pada pin `DC+`-nya.
    *   **Periksa Pengaturan Jumper Relay:** Pastikan jumper `Low - Com - High` pada modul relay diatur ke posisi **`Com - High`**. Ini memastikan relay berfungsi sebagai "HIGH Level Trigger" sesuai dengan kode Anda.
    *   **Periksa Serial Monitor:** Saat Anda mengirim perintah, monitor serial ESP32 seharusnya mencetak pesan `[Pump Control] Pumping X ml...`. Jika tidak, berarti pesan MQTT tidak diterima atau tidak di-parse dengan benar.
*   **Sensor tidak membaca data:**
    *   Periksa kembali semua koneksi kabel, terutama `VCC` dan `GND`.
    *   Gunakan Serial Monitor untuk melihat pesan error spesifik dari sensor.
*   **ESP32 tidak terhubung ke Wi-Fi/MQTT:**
    *   Periksa kembali kredensial Anda di file `credentials.ini`.
    *   Pastikan ESP32 berada dalam jangkauan Wi-Fi dan Broker MQTT Anda dapat diakses.
    *   Gunakan Serial Monitor di PlatformIO untuk melihat log koneksi secara detail.
*   **Perubahan di UI tidak muncul:** Bersihkan cache browser Anda (Ctrl+F5 atau Cmd+Shift+R) dan restart Home Assistant setelah men-deploy perubahan konfigurasi YAML.

## Kontribusi

Kontribusi untuk proyek ini sangat diterima! Jika Anda memiliki saran, perbaikan, atau menemukan bug, jangan ragu untuk membuka *issue* atau mengirimkan *pull request*.

## Lisensi

Proyek ini dilisensikan di bawah Lisensi MIT. Anda bebas menggunakan, memodifikasi, dan mendistribusikan kode ini untuk keperluan pribadi atau komersial, dengan atribusi yang sesuai.
