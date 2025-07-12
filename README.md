# ESP32 Hydroponics Automation System Project

*(Replace with an actual image of your project)*

This project is an automation system for hydroponic cultivation using an ESP32, MQTT, and integration with Home Assistant. The system monitors important parameters such as water level, water temperature, air temperature, humidity, and Total Dissolved Solids (TDS). Furthermore, it provides precise, volume-based control for nutrient and pH pumps, along with an early warning system for critical water levels.

## Table of Contents

* [Main Features](#main-features)
* [Component List](#component-list)
* [Wiring Diagram](#wiring-diagram)
* [Software Preparation](#software-preparation)
* [Usage](#usage)
* [Troubleshooting](#troubleshooting)
* [Contribution](#contribution)
* [License](#license)

## Main Features

*   **Water Level Monitoring:** Uses an ultrasonic sensor to measure the water level in the reservoir and send it to Home Assistant.
*   **Water Temperature Monitoring:** Reads water temperature using a DS18B20 sensor.
*   **Air Temperature & Humidity Monitoring:** Uses a DHT22 sensor to monitor ambient air conditions.
*   **TDS Monitoring:** Measures the Total Dissolved Solids (nutrient concentration) in the water with temperature compensation.
*   **Volume-Based Pump Control:** Precisely controls 3 peristaltic pumps (Nutrient A, Nutrient B, and pH) by volume (ml).
*   **Notifications & Alerts:** Critical water level warning system with MQTT notifications and a buzzer.
*   **Home Assistant Integration:** All sensor data and pump controls are fully integrated with Home Assistant via the MQTT protocol.
*   **MQTT Heartbeat:** Sends periodic "alive" signals to Home Assistant to verify ESP32 connectivity.
*   **Automatic Reconnection:** Automatic Wi-Fi and MQTT reconnection if connections are lost.

## Component List

| Component                     | Quantity | Description                                        |
| :---------------------------- | :------- | :------------------------------------------------- |
| ESP32 Development Board       | 1        | Recommended: NodeMCU ESP32, ESP32 DevKitC          |
| DC 12V 5A Power Supply        | 1        | For powering pumps and the relay module            |
| JSN-SR04T Ultrasonic Sensor   | 1        | For water level measurement                        |
| DS18B20 Temperature Sensor    | 1        | For water temperature                              |
| DHT22 Temperature & Humidity Sensor | 1    | For air temperature & humidity                     |
| Analog TDS Meter              | 1        | For measuring water nutrient concentration         |
| Active Buzzer                 | 1        | For audible alerts                                 |
| 8-Channel Relay Module        | 1        | For pump control           |
| DC 12V 5W Peristaltic Pump    | 3        | Kamoer NKP-DC-504B or similar                      |
| Jumper Wires                  | As needed | Male-to-Male, Female-to-Female, Male-to-Female     |
| Breadboard (optional)         | 1        | For prototyping                                    |
| Resistors (optional)          | As needed for sensors (e.g., DS18B20 pull-up) |
| Enclosure Box (optional)      | 1        | For protection & neat installation                 |

## Wiring Diagram

Here are the detailed pin connections between the ESP32, Sensors, Buzzer, Power Supply, Pumps, and Relay Module.

**Important:** Ensure all `GND` connections are common (`Common Ground`). This means the ESP32's `GND`, the relay module's `DC -`, and the 12V Power Supply's `-V` must be connected to the same point.

### ESP32 Pinout

| ESP32 Pin (GPIO) | Connected Component             | Description                                     |
| :--------------- | :------------------------------ | :---------------------------------------------- |
| 5                | ULTRASONIC_TRIGGER_PIN (JSN-SR04T Trig) | Ultrasonic Sensor Trigger Signal                |
| 4                | ULTRASONIC_ECHO_PIN (JSN-SR04T Echo)   | Ultrasonic Sensor Echo Signal                   |
| 16               | ONE_WIRE_BUS (DS18B20 Data)            | Water Temperature Sensor Data Pin               |
| 13               | DHT_PIN (DHT22 Data)                   | Air Temperature & Humidity Sensor Data Pin      |
| 17               | BUZZER_PIN                      | Buzzer Control Pin                              |
| 34               | TDS_SENSOR_PIN                 | Analog signal from TDS sensor adapter board     |
| 25               | PUMP_NUTRISI_A_PIN (Relay IN1)  | Nutrient Pump A Control                         |
| 26               | PUMP_NUTRISI_B_PIN (Relay IN2)  | Nutrient Pump B Control                         |
| 27               | PUMP_PH_PIN (Relay IN3)         | pH Pump Control                                 |
| GND              | All Component GNDs              | System Common Ground                            |
| 5V (or VIN)      | Relay Module DC+                | Power for Relay Control Circuit                 |

### DC 12V 5A Power Supply Connections

*   `L (AC)` and `N (AC)` Terminals: Connect to your AC power source (e.g., 220V AC).
*   `---` (Earth Ground) Terminal: (Optional) Connect to your building's grounding for safety.
*   `-V` Terminal: Negative DC Output (GND / Common Ground for your 12V DC system).
*   `+V` Terminal: Positive DC Output (+12V).

### 8-Channel Relay Module Connections

**A. Control Side (ESP32 to Relay Module):**
*   **Relay Module `DC -`** --connects to--> **`GND` Pin on your ESP32**.
    *   (Ensure this is also connected to the `-V` of the 12V Power Supply for common ground).
*   **Relay Module `DC +`** --connects to--> **`5V` (or `VIN`) Pin on your ESP32**.
*   **Relay Module `IN1`** --connects to--> **ESP32 GPIO 25 (`PUMP_NUTRISI_A_PIN`)**.
*   **Relay Module `IN2`** --connects to--> **ESP32 GPIO 26 (`PUMP_NUTRISI_B_PIN`)**.
*   **Relay Module `IN3`** --connects to--> **ESP32 GPIO 27 (`PUMP_PH_PIN`)**.

**B. Load Side (12V Power Supply to Relay to Pump):**

*   **For Each Pump (Nutrient A, B, pH):**
    *   **`+V` Terminal (+12V)** from your 12V Power Supply --connects to--> **`COM`** terminal on the relay channel you are using (e.g., `COM1`, `COM2`, `COM3`).
    *   **`NO` (Normally Open)** terminal on the relay channel you are using (e.g., `NO1`, `NO2`, `NO3`) --connects to--> **Positive (+) Wire of your Pump**.
    *   **Negative (-) Wire of your Pump** --connects directly to--> **`-V` Terminal (GND) of your 12V Power Supply**.

**C. Jumper Settings on the Relay Module:**

*   **Crucial:** On your relay module, move the `Low - Com - High` jumper to connect the **`Com` pin with the `High` pin** for each channel you are using (e.g., channels 1, 2, and 3). This setting makes the relay a "HIGH Level Trigger", which matches your code.

## Software Preparation

### Arduino IDE

1.  **Install ESP32 Board Manager:**

    *   Open Arduino IDE.
    *   Go to `File > Preferences`.
    *   In "Additional Boards Manager URLs", add the following URL:
        `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
    *   Go to `Tools > Board > Boards Manager...`.
    *   Search for and install "esp32 by Espressif Systems".

2.  **Install Libraries:**

    *   Go to `Sketch > Include Library > Manage Libraries...`.
    *   Search for and install the following libraries:
        *   `PubSubClient by Nick O'Leary`
        *   `NewPing by Tim Eckel`
        *   `OneWire by Paul Stoffregen`
        *   `DallasTemperature by Miles Burton`
        *   `Adafruit Unified Sensor by Adafruit`
        *   `DHT sensor library by Adafruit`

3.  **Configure Project Code:**
    *   Open the `src/config.cpp` file. This is the central place for all user-specific settings.
    *   **Adjust Wi-Fi Configuration:**
        ```cpp
        const char *WIFI_SSID = "Your_WiFi_SSID";
        const char *WIFI_PASSWORD = "Your_WiFi_Password";
        ```
    *   **Adjust MQTT Broker Configuration:**
        ```cpp
        const char *MQTT_SERVER = "your_mqtt_broker_address"; // e.g., 192.168.1.100
        const int MQTT_PORT = 1883;
        const char *MQTT_USERNAME = "your_mqtt_username";
        const char *MQTT_PASSWORD = "your_mqtt_password";
        ```
    *   **Adjust System & Sensor Calibration:** Modify values like `MQTT_CLIENT_ID`, `TANDON_MAX_HEIGHT_CM`, `WATER_LEVEL_CRITICAL_CM`, and `PUMP_MS_PER_ML` as needed for your specific hardware setup.
    *   Save changes.

4.  **Upload Code:**

    *   Select your ESP32 board from `Tools > Board > ESP32 Arduino`.
    *   Select the correct Serial port from `Tools > Port`.
    *   Upload the code to your ESP32.

### Home Assistant

1.  **MQTT Broker Configuration:**

    *   Ensure you have an MQTT Broker (e.g., Mosquitto Broker Add-on) installed and configured in your Home Assistant.

2.  **Pump Control in Home Assistant:**

    *   Since the pumps are now controlled by volume (e.g., "pump 50 ml") instead of a simple "ON/OFF", standard MQTT switches are no longer suitable. The recommended approach is to use `input_number` helpers and `scripts` to send specific volumes.
    *   Add the following to your Home Assistant's `configuration.yaml` (or dedicated package files).

    *   **a. Create `input_number` helpers to select the volume:**
        ```yaml
        # configuration.yaml
        input_number:
          pump_a_volume:
            name: Nutrient A Dosing Volume
            initial: 10
            min: 1
            max: 200
            step: 1
            unit_of_measurement: "ml"
            icon: mdi:beaker-plus-outline

          pump_b_volume:
            name: Nutrient B Dosing Volume
            initial: 10
            min: 1
            max: 200
            step: 1
            unit_of_measurement: "ml"
            icon: mdi:beaker-plus-outline
        ```

    *   **b. Create `scripts` to trigger the pumps with the selected volume:**
        ```yaml
        # configuration.yaml
        script:
          dose_nutrient_a:
            alias: "Dose Nutrient A"
            icon: mdi:water-pump
            sequence:
              - service: mqtt.publish
                data:
                  topic: "hidroponik/pompa/nutrisi_a/kontrol"
                  payload_template: "{{ states('input_number.pump_a_volume') }}"
          # Create similar scripts for pump_b and pump_ph
        ```

3.  **MQTT Sensor Configuration:**

    *   Add the sensor configuration to your `configuration.yaml` or an included `mqtt_sensors.yaml` file:
        ```yaml
        # Example for mqtt_sensors.yaml
        - platform: mqtt
          name: "Reservoir Water Level"
          state_topic: "hidroponik/air/level_cm"
          unit_of_measurement: "cm"
          icon: mdi:waves-arrow-up
          value_template: "{{ value | float(0) }}"

        - platform: mqtt
          name: "Water Sensor Distance"
          state_topic: "hidroponik/air/jarak_sensor_cm"
          unit_of_measurement: "cm"
          icon: mdi:arrow-expand-vertical
          value_template: "{{ value | float(0) }}"

        - platform: mqtt
          name: "Water Temperature"
          state_topic: "hidroponik/air/suhu_c"
          unit_of_measurement: "°C"
          device_class: temperature
          value_template: "{{ value | float(2) }}"

        - platform: mqtt
          name: "Air Temperature"
          state_topic: "hidroponik/udara/suhu_c"
          unit_of_measurement: "°C"
          device_class: temperature
          value_template: "{{ value | float(2) }}"

        - platform: mqtt
          name: "Air Humidity"
          state_topic: "hidroponik/udara/kelembaban_persen"
          unit_of_measurement: "%"
          device_class: humidity
          value_template: "{{ value | float(2) }}"

        - platform: mqtt
          name: "Water TDS"
          state_topic: "hidroponik/air/tds_ppm"
          unit_of_measurement: "ppm"
          icon: mdi:water-opacity
          value_template: "{{ value | float(2) }}"

        - platform: mqtt
          name: "ESP32 Hydroponics Status"
          state_topic: "tele/esp32hidro/LWT"
          value_template: "{{ value }}"
          icon: mdi:lan-connect

        - platform: mqtt
          name: "Hydroponics Alert"
          state_topic: "hidroponik/peringatan"
          value_template: "{{ value }}"
          icon: mdi:alert
        ```

    *   **c. Create a `select` entity to control the system mode:**
        ```yaml
        # configuration.yaml
        select:
          - name: "Hydroponics System Mode"
            unique_id: hydroponics_system_mode
            state_topic: "hidroponik/sistem/mode/status"
            command_topic: "hidroponik/sistem/mode/kontrol"
            options:
              - "NUTRITION"
              - "CLEANER"
            optimistic: false
            retain: true
        ```

4.  **Restart Home Assistant:** After adding all configurations, restart Home Assistant for the changes to take effect.

## Usage

Once everything is assembled and configured:

1.  Power on your 12V DC Power Supply.
2.  The ESP32 will attempt to connect to Wi-Fi and then to your MQTT Broker.
3.  Sensor data will begin to publish to Home Assistant every 5 seconds.
4.  You can control the pumps by sending a numeric payload (e.g., `50` for 50ml) to the pump's `kontrol` topic. This can be done via Home Assistant scripts (as configured above) or any MQTT client.
5.  To perform an emergency stop on a running pump, send the payload `OFF`.
6.  If the reservoir water level reaches a critical threshold (20 cm or less), the system will send an MQTT alert and activate the buzzer.

## Troubleshooting

*   **Pumps not activating after sending a volume command:**
    *   **Check Wiring Connections:** Ensure all wires are securely and correctly connected, especially `GND`, which must be common to all components (ESP32, Relay, Power Supply).
    *   **Check Relay Power:** Ensure the relay module is receiving sufficient and stable 5V power on its `DC+` pin.
    *   **Check Relay Jumper Settings:** Ensure the `Low - Com - High` jumper on the relay module is set to the **`Com - High`** position. This ensures the relay functions as a "HIGH Level Trigger" according to your code.
    *   **Check Serial Monitor:** When you send a command, the ESP32 serial monitor should print a `[Pump Control] Pumping X ml...` message. If it doesn't, the MQTT message is not being received or parsed correctly.
*   **Sensors not reading data:**
    *   Double-check the data pin connections of the sensors to the ESP32.
    *   Ensure the sensor libraries are correctly installed.
*   **ESP32 not connecting to Wi-Fi/MQTT:**
    *   Double-check your credentials in `src/config.cpp`.
    *   Ensure the ESP32 is within Wi-Fi range and your MQTT Broker is accessible.
    *   Use the Arduino IDE Serial Monitor to view connection logs.

## Contribution

Contributions to this project are highly welcome! If you have suggestions, improvements, or find any bugs, please feel free to open an issue or submit a pull request.

## License

This project is licensed under the MIT License. You are free to use, modify, and distribute this code for personal or commercial purposes, with appropriate attribution.