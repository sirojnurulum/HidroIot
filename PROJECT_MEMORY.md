# HidroIot Project - Long Term Memory & Development Guide

## Project Overview

**HidroIot** is a comprehensive ESP32-based hydroponics automation system that provides complete monitoring and control capabilities for hydroponic cultivation. The system integrates with Home Assistant via MQTT for remote monitoring and control.

### Core Purpose
- Automated monitoring of critical hydroponics parameters (water level, temperature, pH, TDS, etc.)
- Precise pump control for nutrient dosing and irrigation
- Smart alerting system for critical conditions
- Full Home Assistant integration for remote management
- Modular architecture supporting multiple greenhouse instances

## Architecture & Design Philosophy

### Modular Code Structure
The project follows a clean modular architecture with clear separation of concerns:

```
src/
├── main.cpp           # Main entry point and orchestration
├── config.h/cpp       # Centralized configuration and constants
├── sensors.h/cpp      # All sensor reading logic
├── actuators.h/cpp    # Pump and buzzer control logic
├── mqtt_handler.h/cpp # MQTT communication handling
└── ha_config/         # Home Assistant configuration templates
```

### Two-Box Physical Architecture
The system is designed to be split into two enclosures for safety and modularity:

**Box 1 (Controller Box - Dry Area):**
- ESP32 Development Board
- PZEM-004T Power Sensor  
- 8-Channel Relay Module
- Active Buzzer
- DHT22 Air Temperature & Humidity Sensor

**Box 2 (Sensor Box - Near Water):**
- JSN-SR04T Ultrasonic Water Level Sensor
- DS18B20 Water Temperature Sensor
- Analog TDS Sensor
- PH-4502C pH Sensor

**Connection:** Single CAT5e/CAT6 LAN cable (up to 2-3 meters) connects the boxes using T568B standard.

### Multi-Instance Support
The project is designed to support multiple greenhouse instances:
- Instance ID defined at compile time via `HYDROPONIC_INSTANCE_ID`
- All MQTT topics and configurations are instance-specific
- Easy to scale by copying configuration files and changing instance IDs

## Technical Implementation

### ESP32 Pin Configuration
```
GPIO 5  - Ultrasonic Trigger (JSN-SR04T)
GPIO 4  - Ultrasonic Echo (JSN-SR04T)
GPIO 18 - DS18B20 Water Temperature (One-Wire)
GPIO 13 - DHT22 Air Temp/Humidity
GPIO 19 - Buzzer Control
GPIO 34 - TDS Sensor (Analog)
GPIO 32 - pH Sensor (Analog)
GPIO 16 - PZEM-004T RX (connects to PZEM TX)
GPIO 17 - PZEM-004T TX (connects to PZEM RX)
GPIO 25 - Nutrient Pump A (Relay IN1)
GPIO 26 - Nutrient Pump B (Relay IN2)
GPIO 27 - pH Pump (Relay IN3)
GPIO 33 - Watering Pump (Relay IN4)
GPIO 15 - Reservoir Refill Pump (Relay IN5)
```

### Sensor Capabilities
1. **Water Level Monitoring** - JSN-SR04T ultrasonic sensor
2. **Water Temperature** - DS18B20 digital sensor
3. **Air Temperature & Humidity** - DHT22 sensor
4. **TDS (Total Dissolved Solids)** - Analog sensor with temperature compensation
5. **pH Monitoring** - PH-4502C analog sensor with calibration
6. **Power Monitoring** - PZEM-004T (voltage, current, power, energy, frequency, power factor)

### Actuator Control System
- **Volume-based Control:** Nutrient pumps A, B, and pH pump (precision dosing in ml)
- **Duration-based Control:** Watering pump (time-based operation in seconds)
- **Binary Control:** Reservoir refill pump/valve (ON/OFF)
- **Emergency Stop:** All pumps support immediate stop via MQTT "OFF" command
- **Alert System:** Buzzer activation for critical water levels

### MQTT Integration
**Topic Structure:** `hidroiot/{instance_id}/...`

**Sensor Topics (State):**
- `/air/level_cm` - Water level
- `/air/jarak_sensor_cm` - Raw distance
- `/air/suhu_c` - Water temperature  
- `/udara/suhu_c` - Air temperature
- `/udara/kelembaban_persen` - Air humidity
- `/air/tds_ppm` - TDS concentration
- `/air/ph` - pH value
- `/listrik/*` - Power monitoring data

**Control Topics:**
- `/pompa/nutrisi_a/set` - Nutrient pump A commands
- `/pompa/nutrisi_b/set` - Nutrient pump B commands
- `/pompa/ph/set` - pH pump commands
- `/pompa/siram/set` - Watering pump commands
- `/pompa/tandon/set` - Reservoir refill commands
- `/system/mode/set` - System mode (NUTRITION/CLEANER)

### Configuration Management
- **Credentials:** Separated in `credentials.ini` (not committed to git)
- **Instance Configuration:** Compile-time via PlatformIO environments
- **Sensor Calibration:** Centralized constants in `config.cpp`
- **MQTT Topics:** Dynamically constructed from base topic + instance ID

## Hardware Specifications

### Required Components
- ESP32 Development Board (NodeMCU ESP32 or DevKitC recommended)
- DC 12V 5A Power Supply
- 8-Channel Relay Module (HIGH level trigger)
- 4x DC 12V Pumps (3x peristaltic for dosing, 1x standard for watering)
- JSN-SR04T Ultrasonic Sensor
- DS18B20 Temperature Sensor
- DHT22 Temperature & Humidity Sensor
- Analog TDS Meter
- PH-4502C pH Sensor
- PZEM-004T v4 Power Sensor
- Active Buzzer
- Jumper wires and enclosures

### Critical Wiring Notes
- **Common Ground:** ESP32 GND, relay module DC-, and 12V power supply -V must be connected
- **Relay Jumpers:** Set to "Com-High" position for HIGH level triggering
- **Power Distribution:** 12V for pumps, 5V for relay control, 3.3V for analog sensors
- **LAN Cable Wiring:** Follow T568B standard for inter-box connections

## Software Development Environment

### PlatformIO Configuration
- Platform: `espressif32`
- Board: `esp32doit-devkit-v1`
- Framework: `arduino`
- Monitor Speed: `115200`

### Dependencies
```ini
lib_deps =
  knolleary/PubSubClient@^2.8.0      # MQTT client
  teckel12/NewPing@^1.9.7            # Ultrasonic sensor
  paulstoffregen/OneWire@^2.3.8      # DS18B20 communication
  milesburton/DallasTemperature@^4.0.4 # DS18B20 library
  adafruit/Adafruit Unified Sensor@^1.1.15
  adafruit/DHT sensor library@^1.4.6  # DHT22 sensor
  mandulaj/PZEM-004T-v30@^1.1.2      # Power sensor
```

### Build Flags
- `DEBUG_MODE` - Enable detailed serial logging
- Credential injection from `credentials.ini`
- Instance ID definition for multi-instance support

## Home Assistant Integration

### Configuration Structure
```
homeassistant/
├── configuration.yaml      # Main HA config
├── homeassistant.yaml     # Core HA settings
├── http.yaml              # HTTP/proxy settings
├── ui-lovelace.yaml       # Dashboard configuration
└── packages/
    └── greenhouse_a.yaml  # Instance-specific entities
```

### Entity Types
- **Sensors:** All monitoring data (water level, temperature, pH, TDS, power)
- **Input Numbers:** Volume controls for dosing pumps
- **Target Values:** Min/max thresholds for TDS and pH
- **Buttons:** Manual pump controls
- **Alerts:** Critical water level warnings

### Deployment
- Automated deployment via `Makefile`
- Template system for multi-instance configurations
- Remote server synchronization with rsync

## Key Design Patterns

### Non-blocking Operations
- All timing operations use `millis()` for non-blocking execution
- Pump operations are timed and automatically stop
- Sensor readings and MQTT operations don't block main loop

### State Management
- Centralized sensor data structure (`SensorValues`)
- Pump state tracking with automatic timeout
- System mode switching (NUTRITION/CLEANER)
- Alert state management to prevent spam

### Error Handling
- Automatic Wi-Fi and MQTT reconnection
- Sensor reading validation (NaN checks)
- Emergency stop capabilities for all actuators
- Comprehensive logging with debug levels

## Calibration & Maintenance

### Sensor Calibration Required

#### 1. pH Sensor Calibration (CRITICAL)
**Location:** `/calibration/ph_calibration.cpp` dan panduan lengkap di `/calibration/README_KALIBRASI_PH.md`

**Proses:**
- Gunakan larutan buffer pH 4.0 dan pH 7.0
- Upload program kalibrasi khusus (`calibration/platformio.ini`)
- Ikuti step-by-step guide untuk pengukuran 100 sample per titik
- Update konstanta `PH_CALIBRATION_VOLTAGE_4` dan `PH_CALIBRATION_VOLTAGE_7` di `config.cpp`

**Frekuensi:**
- Sensor baru: Setiap 2 minggu (2 bulan pertama)
- Sensor stabil: Setiap 1-3 bulan
- Setelah penggantian sensor: Immediately

**Hardware yang dibutuhkan:**
- Larutan buffer pH 4.0 dan pH 7.0 (fresh)
- Air destilasi untuk pembilasan
- Wadah kecil dan tissue bersih

#### 2. TDS Sensor Calibration
**Parameter:** K-value adjustment (`TDS_K_VALUE` in config.cpp)
- Default: 635.40 (may need adjustment)
- Test dengan larutan TDS standard atau conductivity meter

#### 3. Pump Flow Rate Calibration  
**Parameter:** `PUMP_MS_PER_ML` timing calibration
- Default: 3900 ms/ml (13 sec/rev, 200ml/rev)
- Measure actual ml/minute flow rate untuk setiap pump
- Calibrate volume dispensing accuracy

### Critical Parameters to Monitor
- Water level critical threshold: 20cm (adjustable)
- Pump calibration: 3900 ms/ml default (needs calibration)
- Sensor publish interval: 5 seconds
- Heartbeat interval: 10 seconds

## Development Guidelines

### Adding New Features
1. **New Sensors:** Add to `SensorValues` struct, implement in `sensors.cpp`
2. **New Actuators:** Add to pumps array in `actuators.cpp`, define MQTT topics
3. **New Instances:** Copy environment in `platformio.ini`, create new package file
4. **Configuration Changes:** Update both `config.h` declarations and `config.cpp` definitions

### Code Quality Standards
- Comprehensive documentation with Doxygen-style comments
- Modular design with clear separation of concerns
- Error handling and logging throughout
- Non-blocking code patterns
- Consistent naming conventions

### Testing Approach
- Serial monitor logging for debugging
- MQTT message validation
- Sensor reading verification
- Pump operation timing tests
- Home Assistant entity verification

## Troubleshooting Common Issues

### Pump Not Activating
1. Check wiring connections (especially common ground)
2. Verify relay module jumper settings (Com-High)
3. Confirm 5V power to relay module
4. Monitor serial output for command reception
5. Test MQTT message parsing

### Sensor Reading Issues
1. Verify pin connections
2. Check power supply to sensors
3. Validate sensor libraries installation
4. Test individual sensor readings
5. Check for electrical interference

### Connectivity Problems
1. Validate credentials in `credentials.ini`
2. Check Wi-Fi signal strength
3. Verify MQTT broker accessibility
4. Monitor connection logs via serial

### Home Assistant Integration
1. Verify MQTT broker configuration
2. Check entity discovery in HA
3. Validate topic names and payloads
4. Test manual MQTT commands

## Future Enhancement Opportunities

### Hardware Expansions
- Additional sensor types (conductivity, dissolved oxygen)
- Camera integration for visual monitoring
- Multiple reservoir support
- Automated mixing system

### Software Enhancements
- Web-based configuration interface
- Data logging and analytics
- Automated scheduling system
- Machine learning for optimal dosing

### Infrastructure Improvements
- Distributed sensor nodes (ESP8266 in sensor box)
- Redundant communication paths
- Cloud integration options
- Mobile app development

## Project Status & Next Steps

### Current State
- Core functionality implemented and tested
- Home Assistant integration complete
- Documentation comprehensive
- Multi-instance support ready

### Immediate Development Priorities
1. Sensor calibration procedures
2. Performance optimization
3. Extended testing under real conditions
4. Additional safety features

### Long-term Roadmap
1. Scale to multiple greenhouse instances
2. Advanced automation algorithms  
3. Integration with external systems
4. Commercial deployment considerations

---

This memory file should be updated as the project evolves. Key areas to maintain:
- Configuration changes
- New hardware integrations
- Software architecture updates
- Calibration procedures
- Troubleshooting solutions
- Performance optimizations

Last Updated: August 9, 2025
