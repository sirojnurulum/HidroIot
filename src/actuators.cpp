#include "actuators.h"
#include "config.h"
#include "mqtt_handler.h" // For publishing alerts and state

// --- Module-Private (Static) Variables ---

/**
 * @struct Pump
 * @brief Holds all state and configuration for a single peristaltic pump.
 *        Menyimpan semua status dan konfigurasi untuk satu pompa peristaltik.
 */
struct Pump {
  const int pin;              // GPIO pin number / Nomor pin GPIO
  const char* name;           // Human-readable name / Nama yang mudah dibaca
  const char* commandTopic;   // MQTT topic to control this pump / Topik MQTT untuk mengontrol pompa ini
  const char* stateTopic;     // MQTT topic to report state / Topik MQTT untuk melaporkan status
  bool isOn;                  // Current state (on/off) / Status saat ini (on/off)
  unsigned long stopTime;     // Time (in millis) when the pump should stop / Waktu (dalam milidetik) pompa harus berhenti
};

// Create an array of all pumps for easy, scalable management
// Buat array dari semua pompa untuk manajemen yang mudah dan skalabel
static Pump pumps[] = {
  {PUMP_NUTRISI_A_PIN, "Nutrisi A", COMMAND_TOPIC_PUMP_A, STATE_TOPIC_PUMP_A, false, 0},
  {PUMP_NUTRISI_B_PIN, "Nutrisi B", COMMAND_TOPIC_PUMP_B, STATE_TOPIC_PUMP_B, false, 0},
  {PUMP_PH_PIN, "pH", COMMAND_TOPIC_PUMP_PH, STATE_TOPIC_PUMP_PH, false, 0}
};
// Calculate the number of pumps automatically
// Hitung jumlah pompa secara otomatis
const int NUM_PUMPS = sizeof(pumps) / sizeof(pumps[0]);

// Variabel ini juga dipindahkan ke sini dari main.cpp
// This variable is also moved here from main.cpp
static bool isWaterLevelAlertActive = false;

// State for auto-dosing logic to prevent rapid, repeated dosing
// Status untuk logika dosis otomatis untuk mencegah dosis berulang yang cepat
static unsigned long lastAutoDoseCheckTime = 0;

// State machine for sequential A/B nutrient auto-dosing
// State machine untuk dosis otomatis nutrisi A/B secara berurutan
enum AutoDoseState {
  IDLE,
  WAITING_FOR_B_DOSE
};
static AutoDoseState autoDoseState = IDLE;

// System operating mode, defaults to NUTRITION on startup
// Mode operasi sistem, default ke NUTRITION saat startup
enum SystemMode {
  NUTRITION, // Full functionality with auto-dosing / Fungsionalitas penuh dengan dosis otomatis
  CLEANER    // Auto-dosing is disabled for system flushing / Dosis otomatis dinonaktifkan untuk pembersihan sistem
};
static SystemMode currentSystemMode = NUTRITION;


// --- Forward Declarations for Static Functions ---
static void control_pump(Pump& pump, float volume_ml);
static bool are_any_pumps_running();

/**
 * @brief Menginisialisasi pin aktuator, mengatur pinMode dan memastikan semua OFF saat startup.
 * 
 * @brief Initializes actuator pins, setting their pinMode and ensuring all are OFF at startup.
 */
void actuators_init() {
  LOG_PRINTLN("Initializing actuators...");
  for (int i = 0; i < NUM_PUMPS; i++) {
    pinMode(pumps[i].pin, OUTPUT);
    digitalWrite(pumps[i].pin, LOW); // Ensure all pumps are OFF at startup
  }
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); // Ensure buzzer is OFF
}

/**
 * @brief Loop untuk aktuator, menangani logika non-blocking seperti menghentikan pompa berwaktu.
 * 
 * @brief Loop for actuators, handles non-blocking logic like stopping timed pumps.
 */
void actuators_loop() {
  unsigned long currentTime = millis();
  for (int i = 0; i < NUM_PUMPS; i++) {
    // Check if a timed run for this pump needs to be stopped
    // Periksa apakah pompa berwaktu perlu dihentikan
    if (pumps[i].isOn && pumps[i].stopTime > 0 && currentTime >= pumps[i].stopTime) {
      digitalWrite(pumps[i].pin, LOW);
      pumps[i].isOn = false;
      pumps[i].stopTime = 0; // Clear the stop time
      LOG_PRINTF("[Pump Control] Pump %s finished pumping.\n", pumps[i].name);
      mqtt_publish_state(pumps[i].stateTopic, "OFF", true);

      // If this was part of an auto-dosing sequence, trigger the next part
      // Jika ini adalah bagian dari urutan dosis otomatis, picu bagian berikutnya
      if (autoDoseState == WAITING_FOR_B_DOSE && i == 0) { // Pump A just finished
        LOG_PRINTLN("[Auto-Dose] Dosing Nutrient B as part of sequence.");
        // Safety check before dosing the second part
        if (!are_any_pumps_running()) {
          control_pump(pumps[1], DOSING_AMOUNT_ML); // Dose Nutrient B
        }
        autoDoseState = IDLE; // The sequence is now complete or aborted
      }
    }
  }
}

/**
 * @brief Menangani perintah pompa dari MQTT. Mengurai volume atau perintah OFF.
 * 
 * @brief Handles pump commands from MQTT. Parses volume or OFF commands.
 */
void actuators_handle_pump_command(const char* topic, const String& command) {
  // Find the pump that corresponds to the received topic
  // Temukan pompa yang sesuai dengan topik yang diterima
  for (int i = 0; i < NUM_PUMPS; i++) {
    if (strcmp(topic, pumps[i].commandTopic) == 0) {
      // Handle emergency stop command
      // Tangani perintah berhenti darurat
      if (command == "OFF") {
        digitalWrite(pumps[i].pin, LOW);
        pumps[i].isOn = false;
        pumps[i].stopTime = 0; // Cancel any timed run
        LOG_PRINTF("[Pump Control] Pump %s force stopped.\n", pumps[i].name);
        mqtt_publish_state(pumps[i].stateTopic, "OFF", true);
      } else {
        // Otherwise, treat the command as a volume in ml
        // Jika tidak, perlakukan perintah sebagai volume dalam ml
        float volume_ml = command.toFloat();
        if (volume_ml > 0) {
          control_pump(pumps[i], volume_ml);
        }
      }
      return; // Command handled, no need to check other pumps
    }
  }
}

/**
 * @brief Memproses perintah MQTT yang masuk untuk mengubah mode sistem.
 *        Mendukung mode "NUTRITION" dan "CLEANER".
 * 
 * @brief Processes an incoming MQTT command to change the system mode.
 *        Supports "NUTRITION" and "CLEANER" modes.
 */
void actuators_handle_mode_command(const String& command) {
  String upperCmd = command;
  upperCmd.toUpperCase(); // Case-insensitive comparison

  bool modeChanged = false;
  const char* newModeStr = nullptr;

  if (upperCmd == "NUTRITION") {
    if (currentSystemMode != NUTRITION) {
      currentSystemMode = NUTRITION;
      modeChanged = true;
      newModeStr = "NUTRITION";
    }
  } else if (upperCmd == "CLEANER") {
    if (currentSystemMode != CLEANER) {
      currentSystemMode = CLEANER;
      modeChanged = true;
      newModeStr = "CLEANER";
    }
  } else {
    LOG_PRINTF("[Mode] Received unknown mode command: %s\n", command.c_str());
    return;
  }

  if (modeChanged) {
    LOG_PRINTF("[Mode] System mode changed to %s\n", newModeStr);
    mqtt_publish_state(STATE_TOPIC_SYSTEM_MODE, newModeStr, true);
  } else {
    LOG_PRINTF("[Mode] System already in %s mode.\n", upperCmd.c_str());
  }
}

/**
 * @brief Memperbarui status peringatan (buzzer & MQTT) berdasarkan level air.
 * 
 * @brief Updates the alert status (buzzer & MQTT) based on water level.
 */
void actuators_update_alert_status(const SensorValues& values) {
  // This logic is moved directly from main.cpp's updateBuzzerAlert()
  // Logika ini dipindahkan langsung dari updateBuzzerAlert() di main.cpp
  bool shouldAlertBeActive = false;
  if (isnan(values.waterLevelCm)) {
    shouldAlertBeActive = true; // Alert if sensor reading is invalid / Beri peringatan jika pembacaan sensor tidak valid
  } else if (values.waterLevelCm <= WATER_LEVEL_CRITICAL_CM) {
    shouldAlertBeActive = true; // Alert if water level is critical / Beri peringatan jika level air kritis
  }

  // Send notification only when the alert status changes
  // Kirim notifikasi hanya saat status peringatan berubah
  if (shouldAlertBeActive && !isWaterLevelAlertActive) {
    isWaterLevelAlertActive = true;
    char alertMessage[60];
    if (isnan(values.waterLevelCm)) {
      strcpy(alertMessage, "ALERT: Level air tidak terdeteksi atau di luar jangkauan!");
    } else {
      sprintf(alertMessage, "ALERT: Level air kritis! %.1f cm", values.waterLevelCm);
    }
    mqtt_publish_alert(alertMessage);
    Serial.printf(">>> Peringatan: %s <<<\n", alertMessage);
  } else if (!shouldAlertBeActive && isWaterLevelAlertActive) {
    isWaterLevelAlertActive = false;
    mqtt_publish_alert("Level air normal kembali.");
    Serial.println("Level air kembali normal.");
  }

  // Control the physical buzzer based on the alert status
  // Kontrol buzzer fisik berdasarkan status peringatan
  digitalWrite(BUZZER_PIN, isWaterLevelAlertActive ? HIGH : LOW);
}

/**
 * @brief Mempublikasikan status awal semua aktuator ke MQTT.
 * 
 * @brief Publishes the initial state of all actuators to MQTT.
 */
void actuators_publish_states() {
  LOG_PRINTLN("[MQTT] Publishing initial actuator states...");
  for (int i = 0; i < NUM_PUMPS; i++) {
    mqtt_publish_state(pumps[i].stateTopic, pumps[i].isOn ? "ON" : "OFF", true);
  }
  // Publish initial system mode
  mqtt_publish_state(STATE_TOPIC_SYSTEM_MODE, currentSystemMode == NUTRITION ? "NUTRITION" : "CLEANER", true);
}

/**
 * @brief Memeriksa nilai sensor dan melakukan dosis nutrisi otomatis jika diperlukan.
 *        Menggunakan state machine untuk memastikan Nutrisi A dan B didosis secara berurutan.
 * 
 * @brief Checks sensor values and performs automatic nutrient dosing if required.
 *        Uses a state machine to ensure Nutrient A and B are dosed sequentially.
 * @param values Struct SensorValues yang berisi data sensor terbaru. / The SensorValues struct containing the latest sensor data.
 */
void actuators_auto_dose_nutrients(const SensorValues& values) {
  unsigned long currentTime = millis();

  // 1. Check if the system is in the correct mode for auto-dosing
  // 1. Periksa apakah sistem dalam mode yang benar untuk dosis otomatis
  if (currentSystemMode != NUTRITION) {
    // LOG_PRINTLN("[Auto-Dose] Skipping check, system is in CLEANER mode."); // Uncomment for very verbose logging
    return;
  }

  // 2. Check if enough time has passed since the last check
  // 2. Periksa apakah sudah cukup waktu berlalu sejak pengecekan terakhir
  if (currentTime - lastAutoDoseCheckTime < AUTO_DOSING_CHECK_INTERVAL_MS) {
    return;
  }
  lastAutoDoseCheckTime = currentTime;

  // 3. Safety Check: Don't auto-dose if any pumps are already running or if a sequence is in progress
  // 3. Pemeriksaan Keamanan: Jangan lakukan dosis otomatis jika ada pompa yang sudah berjalan atau jika urutan sedang berlangsung
  if (are_any_pumps_running() || autoDoseState != IDLE) {
    LOG_PRINTLN("[Auto-Dose] Skipping check, a pump is running or a sequence is in progress.");
    return;
  }

  // 4. Check if TDS reading is valid and below the threshold
  // 4. Periksa apakah pembacaan TDS valid dan di bawah ambang batas
  if (isnan(values.tdsPpm)) {
    LOG_PRINTLN("[Auto-Dose] Skipping check, TDS value is invalid (NAN).");
    return;
  }

  if (values.tdsPpm < TDS_LOWER_THRESHOLD_PPM) {
    LOG_PRINTF("[Auto-Dose] TDS is low (%.1f ppm). Starting nutrient dosing sequence.\n", values.tdsPpm);
    autoDoseState = WAITING_FOR_B_DOSE;      // Set state to indicate a sequence has started
    control_pump(pumps[0], DOSING_AMOUNT_ML); // Start the sequence by dosing Nutrient A
  } else {
    LOG_PRINTF("[Auto-Dose] TDS level is OK (%.1f ppm).\n", values.tdsPpm);
  }
}

// --- Static (Private) Function Implementations ---

/**
 * @brief Helper function to check if any pump is currently running.
 *        This is useful to prevent auto-dosing while a manual pump command is active.
 * 
 * @brief Fungsi pembantu untuk memeriksa apakah ada pompa yang sedang berjalan.
 *        Berguna untuk mencegah dosis otomatis saat perintah pompa manual sedang aktif.
 * @return true if any pump is on, false otherwise.
 */
static bool are_any_pumps_running() {
  for (int i = 0; i < NUM_PUMPS; i++) {
    if (pumps[i].isOn) {
      return true;
    }
  }
  return false;
}

/**
 * @brief Activates a specific pump for a calculated duration based on volume.
 * 
 * @brief Mengaktifkan pompa tertentu untuk durasi yang dihitung berdasarkan volume.
 * @param pump A reference to the Pump object to control.
 * @param volume_ml The volume in milliliters to pump.
 */
static void control_pump(Pump& pump, float volume_ml) {
  if (volume_ml <= 0) return;

  // Check if another pump is already running to prevent simultaneous operation
  // Periksa apakah pompa lain sudah berjalan untuk mencegah operasi simultan
  if (are_any_pumps_running()) {
    LOG_PRINTF("[Pump Control] Cannot start pump %s, another pump is already running.\n", pump.name);
    return;
  }

  unsigned long duration_ms = volume_ml * PUMP_MS_PER_ML;
  LOG_PRINTF("[Pump Control] Pumping %.1f ml from %s for %lu ms.\n", volume_ml, pump.name, duration_ms);
  
  pump.stopTime = millis() + duration_ms;
  digitalWrite(pump.pin, HIGH);
  pump.isOn = true;
  mqtt_publish_state(pump.stateTopic, "ON", true);
}