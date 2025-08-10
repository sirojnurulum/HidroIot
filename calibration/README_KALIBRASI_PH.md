# Panduan Kalibrasi Sensor pH PH-4502C (ESP32)

## Program Kalibrasi Sederhana

Gunakan file `ph_calibration_simple.cpp` untuk menampilkan tegangan sensor secara real-time di Serial Monitor. Ikuti langkah berikut untuk kalibrasi manual menggunakan potensiometer.

---

## Langkah Kalibrasi

### Persiapan
- Larutan buffer pH 7.0 dan pH 4.0
- Air destilasi untuk bilas
- Obeng kecil (untuk potensiometer)
- ESP32 terhubung ke komputer

### Proses Kalibrasi

1. **Upload Program**
   - Buka folder `calibration/` di VS Code
   - Build & upload `ph_calibration_simple.cpp` ke ESP32
   - Buka Serial Monitor (baudrate 115200)

2. **Kalibrasi pH 7.0**
   - Celupkan sensor ke larutan pH 7.0
   - Lihat tegangan di Serial Monitor
   - Putar potensiometer hingga tegangan = **2.50V** (±0.05V)

3. **Kalibrasi pH 4.0**
   - Bilas sensor dengan air destilasi
   - Celupkan ke larutan pH 4.0
   - **JANGAN putar potensiometer lagi!**
   - Catat tegangan yang muncul (misal: 2.85V)

4. **Update Konstanta di Program Utama**
   - Edit file `src/config.cpp`:
     ```cpp
     // Update dengan hasil kalibrasi Anda
     const float PH_CALIBRATION_VOLTAGE_7 = 2.500; // Tegangan pH 7.0
     const float PH_CALIBRATION_VOLTAGE_4 = 2.850; // Tegangan pH 4.0 (contoh)
     ```

5. **Upload Program Utama**
   - Build & upload program utama ke ESP32
   - Sensor siap digunakan

---

## Contoh Output Serial
```
Tegangan: 2.501 V | Raw ADC: 3102
Tegangan: 2.499 V | Raw ADC: 3100
Tegangan: 2.502 V | Raw ADC: 3104
```

---

## Tips
- Tunggu sensor stabil 2-3 menit di setiap larutan
- Putar potensiometer perlahan (1/8 putaran)
- Bilas sensor sebelum pindah larutan
- Gunakan larutan buffer yang masih baik

## Troubleshooting
- Tegangan selalu 0V/3.3V: cek kabel, power, dan ground
- Potensiometer tidak berpengaruh: coba putar ke arah sebaliknya
- Pembacaan tidak stabil: tunggu lebih lama, pastikan sensor terendam penuh

---

Setelah kalibrasi, **JANGAN putar potensiometer lagi**. Selamat mencoba!
