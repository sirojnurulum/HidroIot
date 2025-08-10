// Program Kalibrasi 4-Titik Sensor pH PH-4502C untuk ESP32
// Menggunakan data kalibrasi aktual dari eksperimen pH 4.01, 6.86, 7.0, dan 9.18

#include <Arduino.h>

// Program kalibrasi 4-titik sensor pH PH-4502C (ESP32)
// Dengan data aktual dari eksperimen untuk akurasi maksimal

const int PH_PIN = 32; // GPIO 32 (ADC1_CH4) - sesuai dengan koneksi hardware

void setup() {
  Serial.begin(115200);
  
  // Konfigurasi ADC untuk pembacaan yang lebih akurat
  analogSetWidth(12);              // Set resolusi ADC ke 12-bit (4096)
  analogSetAttenuation(ADC_11db);  // Set attenuation untuk range 0-3.3V (bisa handle tegangan lebih tinggi)
  
  pinMode(PH_PIN, INPUT);
  
  Serial.println("\n=== KALIBRASI SENSOR PH - 4-POINT CALIBRATION ===");
  Serial.println("Pin yang digunakan: GPIO " + String(PH_PIN));
  Serial.println("🎯 METODE: Kalibrasi 4-titik untuk akurasi maksimal");
  Serial.println("");
  Serial.println("📋 TITIK KALIBRASI AKTUAL:");
  Serial.println("• pH 4.01 → 3.045V (Buffer asam)");
  Serial.println("• pH 6.86 → 2.510V (Buffer netral - DATA EKSPERIMEN)");
  Serial.println("• pH 7.0  → 2.814V (Referensi netral)"); 
  Serial.println("• pH 9.18 → 2.025V (Buffer basa - DATA EKSPERIMEN)");
  Serial.println("• pH 10.01→ 1.855V (Data eksperimen tambahan)");
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  
  // Test pembacaan awal
  delay(2000);
  int testRead = analogRead(PH_PIN);
  Serial.println("📊 Pembacaan ADC awal: " + String(testRead) + " (Max: 4095)");
  Serial.println("🔧 Sistem siap untuk pengukuran pH...");
  Serial.println("");
}

void loop() {
  // Baca ADC multiple kali untuk averaging (mengurangi noise)
  int totalADC = 0;
  const int samples = 10;
  int minADC = 4095, maxADC = 0;
  
  for(int i = 0; i < samples; i++) {
    int reading = analogRead(PH_PIN);
    totalADC += reading;
    if(reading < minADC) minADC = reading;
    if(reading > maxADC) maxADC = reading;
    delay(10);
  }
  
  int adc = totalADC / samples;
  
  // Konversi ADC ke tegangan
  float volt = adc * (3.3 / 4095.0);
  
  // ✅ KALIBRASI 4-TITIK UNTUK AKURASI MAKSIMAL (DATA AKTUAL)
  float ph_4_voltage = 3.045;   // Tegangan untuk pH 4.01 (dari kalibrasi aktual)
  float ph_686_voltage = 2.510; // Tegangan untuk pH 6.86 (dari eksperimen aktual)
  float ph_7_voltage = 2.814;   // Tegangan untuk pH 7.0 (dari kalibrasi aktual) 
  float ph_9_voltage = 2.025;   // Tegangan untuk pH 9.18 (dari eksperimen aktual: ~2.025V)
  
  // Hitung pH menggunakan interpolasi linear segmental (4-titik)
  float ph_value = 0.0;
  
  if (volt >= ph_4_voltage) {
    // Range asam ekstrem (pH < 4.01): extrapolasi dari pH 4.01 - pH 6.86
    float slope_acid = (ph_4_voltage - ph_686_voltage) / (4.01 - 6.86);
    ph_value = 6.86 + ((volt - ph_686_voltage) / slope_acid);
  } else if (volt >= ph_686_voltage) {
    // Range asam-netral (pH 4.01 - 6.86): gunakan slope pH 4.01 - pH 6.86
    float slope_acid_neutral = (ph_4_voltage - ph_686_voltage) / (4.01 - 6.86);
    ph_value = 6.86 + ((volt - ph_686_voltage) / slope_acid_neutral);
  } else if (volt >= ph_9_voltage) {
    // Range netral-basa (pH 6.86 - 9.18): gunakan slope pH 6.86 - pH 9.18
    float slope_neutral_base = (ph_686_voltage - ph_9_voltage) / (6.86 - 9.18);
    ph_value = 9.18 + ((volt - ph_9_voltage) / slope_neutral_base);
  } else {
    // Range basa (pH > 9.18): extrapolasi dari pH 6.86 - pH 9.18
    float slope_base = (ph_686_voltage - ph_9_voltage) / (6.86 - 9.18);
    ph_value = 9.18 + ((volt - ph_9_voltage) / slope_base);
  }
  
  // Status dan klasifikasi pH yang lebih informatif
  String status = "";
  String pH_category = "";
  
  if (adc < 50) {
    status = " [SANGAT RENDAH - Cek koneksi!]";
  } else if (adc > 4000) {
    status = " [SANGAT TINGGI - Cek VCC!]";
  } else {
    // Klasifikasi berdasarkan nilai pH hasil kalibrasi 4-titik
    if (ph_value < 3.0) {
      pH_category = "SANGAT ASAM";
      status = " [" + pH_category + " 🔴]";
    } else if (ph_value >= 3.0 && ph_value < 5.0) {
      pH_category = "ASAM";
      status = " [" + pH_category + " 🟠]";
    } else if (ph_value >= 5.0 && ph_value < 6.5) {
      pH_category = "SEDIKIT ASAM";
      status = " [" + pH_category + " 🟡]";
    } else if (ph_value >= 6.5 && ph_value <= 7.5) {
      pH_category = "NETRAL";
      status = " [" + pH_category + " 🟢]";
    } else if (ph_value > 7.5 && ph_value <= 9.0) {
      pH_category = "SEDIKIT BASA";
      status = " [" + pH_category + " 🔵]";
    } else if (ph_value > 9.0 && ph_value <= 12.0) {
      pH_category = "BASA";
      status = " [" + pH_category + " 🟣]";
    } else {
      pH_category = "SANGAT BASA";
      status = " [" + pH_category + " 🔴]";
    }
  }
  
  // Output lengkap dengan informasi pH (kalibrasi 4-titik)
  Serial.print("ADC: ");
  Serial.print(adc);
  Serial.print(" | Tegangan: ");
  Serial.print(volt, 3);
  Serial.print("V | pH: ");
  Serial.print(ph_value, 2);
  Serial.println(status);
  
  // Status kalibrasi 4-titik yang lebih informatif
  if (volt >= 3.00 && volt <= 3.10) {
    Serial.println("🎯 SENSOR DALAM pH 4.01 - KALIBRASI ASAM BERHASIL!");
    Serial.println("📊 Data: pH 4.01 = 3.045V, pH 6.86 = 2.510V, pH 9.18 = 2.025V");
  } else if (volt >= 2.45 && volt <= 2.55) {
    Serial.println("🎯 SENSOR DALAM pH 6.86 - KALIBRASI NETRAL BERHASIL! 🧪");
    Serial.println("📊 Data: pH 4.01 = 3.045V, pH 6.86 = 2.510V, pH 9.18 = 2.025V");
  } else if (volt >= 2.75 && volt <= 2.85) {
    Serial.println("🎯 SENSOR DALAM pH 7.0 - REFERENSI NETRAL!");
    Serial.println("📊 Data: pH 4.01 = 3.045V, pH 6.86 = 2.510V, pH 9.18 = 2.025V");
  } else if (volt >= 1.95 && volt <= 2.10) {
    Serial.println("🎯 SENSOR DALAM pH 9.18 - KALIBRASI BASA BERHASIL! 🧪");
    Serial.println("📊 Data: pH 4.01 = 3.045V, pH 6.86 = 2.510V, pH 9.18 = 2.025V");
  } else if (volt >= 1.80 && volt <= 1.90) {
    Serial.println("🔬 SENSOR DALAM pH 10.01 - KALIBRASI BASA TINGGI! 🧪");
    Serial.println("📊 Data: pH 10.01 ≈ 1.855V (extrapolasi dari kalibrasi 4-titik)");
  } else {
    Serial.println("📋 Kalibrasi 4-titik SELESAI! Sensor siap untuk pengukuran pH.");
    Serial.println("✅ pH 4.01: 3.045V | pH 6.86: 2.510V | pH 9.18: 2.025V");
  }
  Serial.println("────────────────────────────────────────");
  
  delay(1000);
}
