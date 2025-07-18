# ESP32 Hydroponics Automation System Project

This project is an automation system for hydroponic cultivation using an ESP32, MQTT, and integration with Home Assistant. The system monitors important parameters such as water level, water temperature, air temperature, humidity, and Total Dissolved Solids (TDS). Furthermore, it provides precise, volume-based control for nutrient and pH pumps, along with an early warning system for critical water levels.

## Table of Contents

*   [Main Features](#main-features)
*   [Multi-Instance Support](#multi-instance-support)
*   [Component List](#component-list)
*   [Wiring Diagram](#wiring-diagram)
*   [Software Preparation](#software-preparation)
*   [Home Assistant Configuration](#home-assistant-configuration)
*   [Usage](#usage)
*   [Troubleshooting](#troubleshooting)
*   [Contribution](#contribution)
*   [License](#license)

## Main Features

*   **Multi-Instance Support:** A single codebase to generate different firmware for two types of installations (`production` and `seeding`).
*   **Water Level Monitoring:** Uses an ultrasonic sensor to measure the water level in the reservoir and send it to Home Assistant.
*   **Water Temperature Monitoring:** Reads water temperature using a DS18B20 sensor.
*   **Air Temperature & Humidity Monitoring:** Uses a DHT22 sensor to monitor ambient air conditions.
*   **TDS Monitoring:** Measures the Total Dissolved Solids (nutrient concentration) in the water with temperature compensation.
*   **Volume-Based Pump Control:** Precisely controls 3 peristaltic pumps (Nutrient A, Nutrient B, and pH) by volume (ml).
*   **Notifications & Alerts:** Critical water level warning system with MQTT notifications and a buzzer.
*   **Home Assistant Integration:** All sensor data and pump controls are fully integrated with Home Assistant via the MQTT protocol.
*   **MQTT Heartbeat:** Sends periodic "alive" signals to Home Assistant to verify ESP32 connectivity.
*   **Automatic Reconnection:** Automatic Wi-Fi and MQTT reconnection if connections are lost.
*   **Secure Credential Management:** Separates Wi-Fi and MQTT credentials from the main code using a `credentials.ini` file.

## Multi-Instance Support

This project is designed to manage two types of hydroponic systems from the same codebase:
1.  **`production` Instance:** A full-featured system, including all sensors, 3 dosing pumps, system modes, and alerts.
2.  **`seeding` Instance:** A simple system with only one function: controlling a watering pump based on duration.

## Component List

| Component                     | Qty (Production) | Qty (Seeding) | Description                                        |
| :---------------------------- | :--------------- | :------------ | :------------------------------------------------- |
| ESP32 Development Board       | 1                | 1             | Recommended: NodeMCU ESP32, ESP32 DevKitC          |
| DC 12V Power Supply           | 1                | 1             | For powering pumps and the relay module            |
| DC 12V Pump                   | 3 (Peristaltic)  | 1 (Any)       | Kamoer NKP (production) or a regular water pump (seeding) |
| Relay Module                  | 1 (min. 3-ch)    | 1 (min. 1-ch) | For pump control                                   |
| JSN-SR04T Ultrasonic Sensor   | 1                | 0             | For water level measurement                        |
| DS18B20 Temperature Sensor    | 1                | 0             | For water temperature                              |
| DHT22 Temp & Humidity Sensor  | 1                | 0             | For air temperature & humidity                     |
| Analog TDS Meter              | 1                | 0             | For measuring water nutrient concentration         |
| Active Buzzer                 | 1                | 0             | For audible alerts                                 |
| Jumper Wires                  | As needed        | As needed     | Male-to-Male, Female-to-Female, Male-to-Female     |
| Enclosure Box (optional)      | 1                | 1             | For protection & a neat installation               |

## Wiring Diagram

Here are the detailed pin connections between the ESP32, Sensors, Buzzer, Power Supply, Pumps, and Relay Module.

**Important:** Ensure all `GND` connections are common (`Common Ground`). This means the ESP32's `GND`, the relay module's `DC -`, and the 12V Power Supply's `-V` must be connected to the same point.

### Pinout for `Production` Instance

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

### Pinout for `Seeding` Instance

| ESP32 Pin (GPIO) | Connected Component           | Description                                     |
| :--------------- | :------------------------------ | :---------------------------------------------- |
| 32               | PUMP_SIRAM_PIN (Relay INx)      | Watering Pump Control                           |
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
*   **Relay Module `DC +`** --connects to--> **`5V` (or `VIN`) Pin on your ESP32**.
*   **Relay Module `IN1`** --connects to--> **ESP32 GPIO 25 (`PUMP_NUTRISI_A_PIN`)**.
*   **Relay Module `IN2`** --connects to--> **ESP32 GPIO 26 (`PUMP_NUTRISI_B_PIN`)**.
*   **Relay Module `IN3`** --connects to--> **ESP32 GPIO 27 (`PUMP_PH_PIN`)**.
*   **Relay Module `INx`** --connects to--> **ESP32 GPIO 32 (`PUMP_SIRAM_PIN`)** (for the seeding instance).

**B. Load Side (12V Power Supply to Relay to Pump):**

*   **For Each Pump:**
    *   **`+V` Terminal (+12V)** from your 12V Power Supply --connects to--> **`COM`** terminal on the relay channel you are using (e.g., `COM1`, `COM2`, `COM3`).
    *   **`NO` (Normally Open)** terminal on the relay channel you are using (e.g., `NO1`, `NO2`, `NO3`) --connects to--> **Positive (+) Wire of your Pump**.
    *   **Negative (-) Wire of your Pump** --connects directly to--> **`-V` Terminal (GND) of your 12V Power Supply**.

**C. Jumper Settings on the Relay Module:**
*   **Crucial:** On your relay module, move the `Low - Com - High` jumper to connect the **`Com` pin with the `High` pin** for each channel you are using. This setting makes the relay a "HIGH Level Trigger", which matches your code.

## Software Preparation (PlatformIO)

This project is designed to be compiled using **PlatformIO** within Visual Studio Code.

1.  **Install VS Code & PlatformIO:**
    *   Download and install Visual Studio Code.
    *   Open VS Code, go to the Extensions tab (square icon), search for, and install the **PlatformIO IDE** extension.

2.  **Open the Project:**
    *   Clone this repository or download it as a ZIP and extract it.
    *   In VS Code, go to `File > Open Folder...` and select the `HidroIot` project directory. PlatformIO will automatically detect the `platformio.ini` file and install all required dependencies.

3.  **Create `credentials.ini` File:**
    *   In the project's root directory (at the same level as `platformio.ini`), create a new file named `credentials.ini`.
    *   Fill this file with your credentials. This file **will not** be uploaded to Git.
        ```ini
        [credentials]
        wifi_ssid = "Your_WiFi_SSID"
        wifi_password = "Your_WiFi_Password"
        mqtt_user = "Your_MQTT_Username"
        mqtt_pass = "Your_MQTT_Password"
        ```

4.  **Select Environment and Upload:**
    *   Connect your ESP32 board to your computer.
    *   At the bottom of the VS Code window, you will see the PlatformIO status bar. Click on the environment name (e.g., `Default (produksi)`).
    *   Choose the environment you want to upload: **`produksi`** or **`penyemaian`**.
    *   Click the **Upload** button (right-arrow icon) in the status bar. PlatformIO will compile and upload the correct firmware to your device.
## Home Assistant Configuration

1.  **MQTT Broker Configuration:**
    *   Ensure you have an MQTT Broker (e.g., Mosquitto Broker Add-on) installed and configured in your Home Assistant.

2.  **Entity Configuration in Home Assistant:**
    *   Since this project now supports multiple instances, you need to add the corresponding YAML configuration for each ESP32 you deploy.
    *   Copy the code blocks below into their respective files (`configuration.yaml`, `input_numbers.yaml`, `scripts.yaml`).

    ---

    ### **Configuration for `PRODUCTION` Instance**

    *   **a. Add to `input_numbers.yaml`:**
        ```yaml
        # Helpers for selecting pump volume (Production Instance)
        produksi_pompa_a_volume:
          name: "Production - Dose Volume A"
          initial: 20
          min: 1
          max: 200
          step: 1
          unit_of_measurement: "ml"
          icon: mdi:beaker-plus-outline
          mode: box

        produksi_pompa_b_volume:
          name: "Production - Dose Volume B"
          initial: 20
          min: 1
          max: 200
          step: 1
          unit_of_measurement: "ml"
          icon: mdi:beaker-plus-outline
          mode: box

        produksi_pompa_ph_volume:
          name: "Production - Dose Volume pH"
          initial: 10
          min: 1
          max: 100
          step: 1
          unit_of_measurement: "ml"
          icon: mdi:ph
          mode: box
        ```

    *   **b. Add to `scripts.yaml`:**
        ```yaml
        # =================================================
        # == SCRIPTS FOR 'PRODUCTION' INSTANCE
        # =================================================
        produksi_dosis_nutrisi_a:
          alias: "Production - Dose Nutrient A"
          icon: mdi:water-pump
          sequence:
            - service: mqtt.publish
              data:
                topic: "hidroponik/produksi/pompa/nutrisi_a/kontrol"
                payload: "{{ states('input_number.produksi_pompa_a_volume') | int(0) }}"
                retain: false

        produksi_dosis_nutrisi_b:
          alias: "Production - Dose Nutrient B"
          icon: mdi:water-pump
          sequence:
            - service: mqtt.publish
              data:
                topic: "hidroponik/produksi/pompa/nutrisi_b/kontrol"
                payload: "{{ states('input_number.produksi_pompa_b_volume') | int(0) }}"
                retain: false

        produksi_dosis_ph:
          alias: "Production - Dose pH"
          icon: mdi:water-pump
          sequence:
            - service: mqtt.publish
              data:
                topic: "hidroponik/produksi/pompa/ph/kontrol"
                payload: "{{ states('input_number.produksi_pompa_ph_volume') | int(0) }}"
                retain: false
        ```

    *   **c. Add to `configuration.yaml` (under `mqtt:`):**
        ```yaml
        # MQTT Configuration (Modern & Complete Format)
        mqtt:
          # =================================================
          # == ENTITIES FOR 'PRODUCTION' INSTANCE
          # =================================================
          sensor:
            - name: "Production - Reservoir Water Level"
              unique_id: hidroponik_produksi_level_air
              state_topic: "hidroponik/produksi/air/level_cm"
              unit_of_measurement: "cm"
              icon: mdi:waves-arrow-up
              value_template: "{{ value | float(0) }}"
              availability_topic: "hidroponik/produksi/status/LWT"
              payload_available: "Online"
              payload_not_available: "Offline"

            - name: "Production - Water Temperature"
              unique_id: hidroponik_produksi_suhu_air
              state_topic: "hidroponik/produksi/air/suhu_c"
              unit_of_measurement: "°C"
              device_class: temperature
              value_template: "{{ value | float(2) }}"
              availability_topic: "hidroponik/produksi/status/LWT"
              payload_available: "Online"
              payload_not_available: "Offline"

            - name: "Production - Air Temperature"
              unique_id: hidroponik_produksi_suhu_udara
              state_topic: "hidroponik/produksi/udara/suhu_c"
              unit_of_measurement: "°C"
              device_class: temperature
              value_template: "{{ value | float(2) }}"
              availability_topic: "hidroponik/produksi/status/LWT"
              payload_available: "Online"
              payload_not_available: "Offline"

            - name: "Production - Air Humidity"
              unique_id: hidroponik_produksi_kelembaban_udara
              state_topic: "hidroponik/produksi/udara/kelembaban_persen"
              unit_of_measurement: "%"
              device_class: humidity
              value_template: "{{ value | float(2) }}"
              availability_topic: "hidroponik/produksi/status/LWT"
              payload_available: "Online"
              payload_not_available: "Offline"

            - name: "Production - Water TDS"
              unique_id: hidroponik_produksi_tds_air
              state_topic: "hidroponik/produksi/air/tds_ppm"
              unit_of_measurement: "ppm"
              icon: mdi:water-opacity
              value_template: "{{ value | float(2) }}"
              availability_topic: "hidroponik/produksi/status/LWT"
              payload_available: "Online"
              payload_not_available: "Offline"

          binary_sensor:
            - name: "Production - ESP32 Status"
              unique_id: hidroponik_produksi_status_online
              state_topic: "hidroponik/produksi/status/LWT"
              payload_on: "Online"
              payload_off: "Offline"
              device_class: connectivity

          # System Mode Control (Production only)
          select:
            - name: "Production - System Mode"
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

    ### **Configuration for `SEEDING` Instance**

    *   **a. Add to `input_numbers.yaml`:**
        ```yaml
        # Helper for watering duration (Seeding Instance)
        penyemaian_pompa_siram_durasi:
          name: "Seeding - Watering Duration"
          initial: 15
          min: 1
          max: 300 # Upper limit of 5 minutes, can be adjusted
          step: 1
          unit_of_measurement: "s"
          icon: mdi:timer-sand
          mode: box
        ```

    *   **b. Add to `scripts.yaml`:**
        ```yaml
        # =================================================
        # == SCRIPT FOR 'SEEDING' INSTANCE
        # =================================================
        siram_penyemaian:
          alias: "Seeding - Water Pump"
          icon: mdi:sprinkler-variant
          sequence:
            - service: mqtt.publish
              data:
                topic: "hidroponik/penyemaian/pompa/siram/kontrol"
                payload: "{{ states('input_number.penyemaian_pompa_siram_durasi') | int(0) }}"
                retain: false
        ```

    *   **c. Add to `configuration.yaml` (under `mqtt: > binary_sensor:`):**
        ```yaml
          binary_sensor:
            # ... (production binary sensor above this line) ...
            # =================================================
            # == ENTITIES FOR 'SEEDING' INSTANCE
            # =================================================
            - name: "Seeding - ESP32 Status"
              unique_id: hidroponik_penyemaian_status_online
              state_topic: "hidroponik/penyemaian/status/LWT"
              payload_on: "Online"
              payload_off: "Offline"
              device_class: connectivity
        ```
## Usage

Once everything is assembled and configured:

1.  Power on your 12V DC Power Supply.
2.  The ESP32 will attempt to connect to Wi-Fi and then to your MQTT Broker.
3.  Sensor data (for the `production` instance) will begin to publish to Home Assistant.
4.  You can control the pumps via the Home Assistant dashboard.
5.  To perform an emergency stop on a running pump, send the payload `OFF` to its control topic.
6.  If the reservoir water level (for the `production` instance) reaches a critical threshold, the system will send an MQTT alert and activate the buzzer.

## Troubleshooting

*   **Pumps not activating after sending a volume command:**
    *   **Check Wiring Connections:** Ensure all wires are securely and correctly connected, especially `GND`, which must be common to all components (ESP32, Relay, Power Supply).
    *   **Check Relay Power:** Ensure the relay module is receiving sufficient and stable 5V power on its `DC+` pin.
    *   **Check Relay Jumper Settings:** Ensure the `Low - Com - High` jumper on the relay module is set to the **`Com - High`** position. This ensures the relay functions as a "HIGH Level Trigger" according to your code.
    *   **Check Serial Monitor:** When you send a command, the ESP32 serial monitor should print a `[Pump Control] Pumping X ml...` message. If it doesn't, the MQTT message is not being received or parsed correctly.
*   **Sensors not reading data (Production Instance):**
    *   Double-check the data pin connections of the sensors to the ESP32.
    *   Ensure the relevant sensor libraries have been correctly installed by PlatformIO.
*   **ESP32 not connecting to Wi-Fi/MQTT:**
    *   Double-check your credentials in the `credentials.ini` file.
    *   Ensure the ESP32 is within Wi-Fi range and your MQTT Broker is accessible.
    *   Use the Serial Monitor in PlatformIO to view detailed connection logs.

## Contribution

Contributions to this project are highly welcome! If you have suggestions, improvements, or find any bugs, please feel free to open an issue or submit a pull request.

## License

This project is licensed under the MIT License. You are free to use, modify, and distribute this code for personal or commercial purposes, with appropriate attribution.
