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
  const std::string commandTopic; // MQTT topic to control this pump / Topik MQTT untuk mengontrol pompa ini
  const std::string stateTopic;   // MQTT topic to report state / Topik MQTT untuk melaporkan status
  bool isOn;                  // Current state (on/off) / Status saat ini (on/off)
  unsigned long stopTime;     // Time (in millis) when the pump should stop / Waktu (dalam milidetik) pompa harus berhenti
};

// Create an array of all pumps for easy, scalable management
// Buat array dari semua pompa untuk manajemen yang mudah dan skalabel
#if HYDROPONIC_INSTANCE == 1 // Produksi
static Pump pumps[] = {
  {PUMP_NUTRISI_A_PIN, "Nutrisi A", COMMAND_TOPIC_PUMP_A, STATE_TOPIC_PUMP_A, false, 0},
  {PUMP_NUTRISI_B_PIN, "Nutrisi B", COMMAND_TOPIC_PUMP_B, STATE_TOPIC_PUMP_B, false, 0},
  {PUMP_PH_PIN, "pH", COMMAND_TOPIC_PUMP_PH, STATE_TOPIC_PUMP_PH, false, 0}
};
#elif HYDROPONIC_INSTANCE == 2 // Penyemaian (Watering Only)
static Pump pumps[] = {
  {PUMP_SIRAM_PIN, "Penyiraman", COMMAND_TOPIC_PUMP_SIRAM, STATE_TOPIC_PUMP_SIRAM, false, 0}
};
#endif
// Calculate the number of pumps automatically
// Hitung jumlah pompa secara otomatis
const int NUM_PUMPS = sizeof(pumps) / sizeof(pumps[0]);

#if HYDROPONIC_INSTANCE == 1
// Variabel ini hanya untuk instance produksi
static bool isWaterLevelAlertActive = false;

// State for auto-dosing logic to prevent rapid, repeated dosing
static unsigned long lastAutoDoseCheckTime = 0;

// State machine for sequential A/B nutrient auto-dosing
enum AutoDoseState {
  IDLE,
  WAITING_FOR_B_DOSE
};
static AutoDoseState autoDoseState = IDLE;

// System operating mode, defaults to NUTRITION on startup
enum SystemMode {
  NUTRITION, // Full functionality with auto-dosing
  CLEANER    // Auto-dosing is disabled for system flushing
};
static SystemMode currentSystemMode = NUTRITION;
#endif


// --- Forward Declarations for Static Functions ---
static void control_pump(Pump& pump, float volume_ml);
static void control_pump_for_duration(Pump& pump, unsigned long duration_ms);
static bool are_any_pumps_running();

/**
 * @brief Menginisialisasi pin aktuator, mengatur pinMode dan memastikan semua OFF saat startup.
 */
void actuators_init() {
  LOG_PRINTLN("Initializing actuators...");
  for (int i = 0; i < NUM_PUMPS; i++) {
    pinMode(pumps[i].pin, OUTPUT);
    digitalWrite(pumps[i].pin, LOW); // Ensure all pumps are OFF at startup
  }
#if HYDROPONIC_INSTANCE == 1
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); // Ensure buzzer is OFF
#endif
}

/**
 * @brief Loop untuk aktuator, menangani logika non-blocking seperti menghentikan pompa berwaktu.
 */
void actuators_loop() {
  unsigned long currentTime = millis();
  for (int i = 0; i < NUM_PUMPS; i++) {
    // Check if a timed run for this pump needs to be stopped
    if (pumps[i].isOn && pumps[i].stopTime > 0 && currentTime >= pumps[i].stopTime) {
      digitalWrite(pumps[i].pin, LOW);
      pumps[i].isOn = false;
      pumps[i].stopTime = 0; // Clear the stop time
      LOG_PRINTF("[Pump Control] Pump %s finished pumping.\n", pumps[i].name);
      mqtt_publish_state(pumps[i].stateTopic.c_str(), "OFF", true);

#if HYDROPONIC_INSTANCE == 1
      // If this was part of an auto-dosing sequence, trigger the next part
      if (autoDoseState == WAITING_FOR_B_DOSE && i == 0) { // Pump A just finished
        LOG_PRINTLN("[Auto-Dose] Dosing Nutrient B as part of sequence.");
        if (!are_any_pumps_running()) {
          control_pump(pumps[1], DOSING_AMOUNT_ML); // Dose Nutrient B
        }
        autoDoseState = IDLE; // The sequence is now complete or aborted
      }
#endif
    }
  }
}

/**
 * @brief Menangani perintah pompa dari MQTT. Mengurai volume atau perintah OFF.
 */
void actuators_handle_pump_command(const char* topic, const String& command) {
  for (int i = 0; i < NUM_PUMPS; i++) {
    if (topic == pumps[i].commandTopic) {
      if (command == "OFF") {
        digitalWrite(pumps[i].pin, LOW);
        pumps[i].isOn = false;
        pumps[i].stopTime = 0; // Cancel any timed run
        LOG_PRINTF("[Pump Control] Pump %s force stopped.\n", pumps[i].name);
        mqtt_publish_state(pumps[i].stateTopic.c_str(), "OFF", true);
#if HYDROPONIC_INSTANCE == 2
      } else if (topic == COMMAND_TOPIC_PUMP_SIRAM) {
        // This is the watering pump, command is duration in seconds
        float duration_s = command.toFloat();
        if (duration_s > 0) {
          unsigned long duration_ms = duration_s * 1000;
          control_pump_for_duration(pumps[i], duration_ms);
        }
#endif
      } else {
        // Otherwise, treat the command as a volume in ml
        float volume_ml = command.toFloat();
        if (volume_ml > 0) {
          control_pump(pumps[i], volume_ml);
        }
      }
      return; // Command handled
    }
  }
}

#if HYDROPONIC_INSTANCE == 1
/**
 * @brief Memproses perintah MQTT yang masuk untuk mengubah mode sistem.
 */
void actuators_handle_mode_command(const String& command) {
  String upperCmd = command;
  upperCmd.toUpperCase();

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
    mqtt_publish_state(STATE_TOPIC_SYSTEM_MODE.c_str(), newModeStr, true);
  } else {
    LOG_PRINTF("[Mode] System already in %s mode.\n", upperCmd.c_str());
  }
}

/**
 * @brief Memperbarui status peringatan (buzzer & MQTT) berdasarkan level air.
 */
void actuators_update_alert_status(const SensorValues& values) {
  bool shouldAlertBeActive = false;
  if (isnan(values.waterLevelCm)) {
    shouldAlertBeActive = true;
  } else if (values.waterLevelCm <= WATER_LEVEL_CRITICAL_CM) {
    shouldAlertBeActive = true;
  }

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

  digitalWrite(BUZZER_PIN, isWaterLevelAlertActive ? HIGH : LOW);
}
#endif

/**
 * @brief Mempublikasikan status awal semua aktuator ke MQTT.
 */
void actuators_publish_states() {
  LOG_PRINTLN("[MQTT] Publishing initial actuator states...");
  for (int i = 0; i < NUM_PUMPS; i++) {
    mqtt_publish_state(pumps[i].stateTopic.c_str(), pumps[i].isOn ? "ON" : "OFF", true);
  }
#if HYDROPONIC_INSTANCE == 1
  // Publish initial system mode
  mqtt_publish_state(STATE_TOPIC_SYSTEM_MODE.c_str(), currentSystemMode == NUTRITION ? "NUTRITION" : "CLEANER", true);
#endif
}

#if HYDROPONIC_INSTANCE == 1
/**
 * @brief Memeriksa nilai sensor dan melakukan dosis nutrisi otomatis jika diperlukan.
 */
void actuators_auto_dose_nutrients(const SensorValues& values) {
  unsigned long currentTime = millis();

  if (currentSystemMode != NUTRITION) {
    return;
  }

  if (currentTime - lastAutoDoseCheckTime < AUTO_DOSING_CHECK_INTERVAL_MS) {
    return;
  }
  lastAutoDoseCheckTime = currentTime;

  if (are_any_pumps_running() || autoDoseState != IDLE) {
    LOG_PRINTLN("[Auto-Dose] Skipping check, a pump is running or a sequence is in progress.");
    return;
  }

  if (isnan(values.tdsPpm)) {
    LOG_PRINTLN("[Auto-Dose] Skipping check, TDS value is invalid (NAN).");
    return;
  }

  if (values.tdsPpm < TDS_LOWER_THRESHOLD_PPM) {
    LOG_PRINTF("[Auto-Dose] TDS is low (%.1f ppm). Starting nutrient dosing sequence.\n", values.tdsPpm);
    autoDoseState = WAITING_FOR_B_DOSE;
    control_pump(pumps[0], DOSING_AMOUNT_ML); // Start with Nutrient A
  } else {
    LOG_PRINTF("[Auto-Dose] TDS level is OK (%.1f ppm).\n", values.tdsPpm);
  }
}
#endif

// --- Static (Private) Function Implementations ---

/**
 * @brief Helper function to check if any pump is currently running.
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
 */
static void control_pump(Pump& pump, float volume_ml) {
  if (volume_ml <= 0) return;

  if (are_any_pumps_running()) {
    LOG_PRINTF("[Pump Control] Cannot start pump %s, another pump is already running.\n", pump.name);
    return;
  }

  unsigned long duration_ms = volume_ml * PUMP_MS_PER_ML;
  LOG_PRINTF("[Pump Control] Pumping %.1f ml from %s for %lu ms.\n", volume_ml, pump.name, duration_ms);
  
  pump.stopTime = millis() + duration_ms;
  digitalWrite(pump.pin, HIGH);
  pump.isOn = true;
  mqtt_publish_state(pump.stateTopic.c_str(), "ON", true);
}

/**
 * @brief Activates a specific pump for a given duration.
 */
static void control_pump_for_duration(Pump& pump, unsigned long duration_ms) {
  if (duration_ms <= 0) return;

  if (are_any_pumps_running()) {
    LOG_PRINTF("[Pump Control] Cannot start pump %s, another pump is already running.\n", pump.name);
    return;
  }

  LOG_PRINTF("[Pump Control] Running %s for %lu ms.\n", pump.name, duration_ms);
  
  pump.stopTime = millis() + duration_ms;
  digitalWrite(pump.pin, HIGH);
  pump.isOn = true;
  mqtt_publish_state(pump.stateTopic.c_str(), "ON", true);
}
