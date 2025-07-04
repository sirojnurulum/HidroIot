# ESP32 Hydroponics Automation System Project

*(Replace with the actual image of your project)*

This project is a basic automation system for hydroponic cultivation using ESP32, MQTT, and integration with Home Assistant. The system is capable of monitoring important parameters such as water level, water temperature, and ambient air temperature and humidity. Furthermore, it provides automated control for nutrient pumps (A & B) and a pH pump, along along with an early warning system for critical water levels.

## Table of Contents

* [Main Features](https://www.google.com/search?q=%23main-features)
* [Component List](https://www.google.com/search?q=%23component-list)
* [Wiring Diagram](https://www.google.com/search?q=%23wiring-diagram)
* [Software Preparation](https://www.google.com/search?q=%23software-preparation)
    * [Arduino IDE](https://www.google.com/search?q=%23arduino-ide)
    * [Home Assistant](https://www.google.com/search?q=%23home-assistant)
* [Assembly & Wiring Instructions](https://www.google.com/search?q=%23assembly--wiring-instructions)
    * [Power Supply](https://www.google.com/search?q=%23power-supply)
    * [JSN-SR04T Ultrasonic Sensor](https://www.google.com/search?q=%23jsn-sr04t-ultrasonic-sensor)
    * [DS18B20 Temperature Sensor](https://www.google.com/search?q=%23ds18b20-temperature-sensor)
    * [DHT22 Temperature & Humidity Sensor](https://www.google.com/search?q=%23dht22-temperature--humidity-sensor)
    * [Buzzer](https://www.google.com/search?q=%23buzzer)
    * [Relay Module (MOSFET Replacement)](https://www.google.com/search?q=%23relay-module-mosfet-replacement)
* [Usage](https://www.google.com/search?q=%23usage)
* [Troubleshooting](https://www.google.com/search?q=%23troubleshooting)
* [Contribution](https://www.google.com/search?q=%23contribution)
* [License](https://www.google.com/search?q=%23license)

## Main Features

* **Water Level Monitoring:** Uses an ultrasonic sensor to measure the water level in the reservoir and send it to Home Assistant.
* **Water Temperature Monitoring:** Reads water temperature using a DS18B20 sensor.
* **Air Temperature & Humidity Monitoring:** Uses a DHT22 sensor to monitor ambient air conditions.
* **Automated Pump Control:** Controls 3 peristaltic pumps (Nutrient A, Nutrient B, and pH) via relays.
* **Notifications & Alerts:** Critical water level warning system with MQTT notifications and a buzzer.
* **Home Assistant Integration:** All sensor data and pump controls are fully integrated with Home Assistant via the MQTT protocol.
* **MQTT Heartbeat:** Sends periodic "alive" signals to Home Assistant to verify ESP32 connectivity.
* **Automatic Reconnection:** Automatic Wi-Fi and MQTT reconnection if connections are lost.

## Component List

| Component                     | Quantity | Description                                        |
| :---------------------------- | :------- | :------------------------------------------------- |
| ESP32 Development Board       | 1        | Recommended: NodeMCU ESP32, ESP32 DevKitC          |
| DC 12V 5A Power Supply        | 1        | For powering pumps and the relay module            |
| JSN-SR04T Ultrasonic Sensor   | 1        | For water level measurement                        |
| DS18B20 Temperature Sensor    | 1        | For water temperature                              |
| DHT22 Temperature & Humidity Sensor | 1    | For air temperature & humidity                     |
| Active Buzzer                 | 1        | For audible alerts                                 |
| 8-Channel Relay Module        | 1        | Replaces MOSFET module, for pump control           |
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
| 5                | ULTRASONIC\_TRIGGER\_PIN (JSN-SR04T Trig) | Ultrasonic Sensor Trigger Signal                |
| 4                | ULTRASONIC\_ECHO\_PIN (JSN-SR04T Echo)   | Ultrasonic Sensor Echo Signal                   |
| 16               | ONE\_WIRE\_BUS (DS18B20 Data)            | Water Temperature Sensor Data Pin               |
| 13               | DHT\_PIN (DHT22 Data)                   | Air Temperature & Humidity Sensor Data Pin      |
| 17               | BUZZER\_PIN                      | Buzzer Control Pin                              |
| 25               | PUMP\_NUTRISI\_A\_PIN (Relay IN1)  | Nutrient Pump A Control                         |
| 26               | PUMP\_NUTRISI\_B\_PIN (Relay IN2)  | Nutrient Pump B Control                         |
| 27               | PUMP\_PH\_PIN (Relay IN3)         | pH Pump Control                                 |
| GND              | All Component GNDs              | System Common Ground                            |
| 5V (or VIN)      | Relay Module DC+                | Power for Relay Control Circuit                 |

### DC 12V 5A Power Supply Connections

* `L (AC)` and `N (AC)` Terminals: Connect to your AC power source (e.g., 220V AC).
* `---` (Earth Ground) Terminal: (Optional) Connect to your building's grounding for safety.
* `-V` Terminal: Negative DC Output (GND / Common Ground for your 12V DC system).
* `+V` Terminal: Positive DC Output (+12V).

### 8-Channel Relay Module Connections

**A. Control Side (ESP32 to Relay Module):**

* **Relay Module `DC -`** --connects to--\> **`GND` Pin on your ESP32**.
    * (Ensure this is also connected to the `-V` of the 12V Power Supply for common ground).
* **Relay Module `DC +`** --connects to--\> **`5V` (or `VIN`) Pin on your ESP32**.
    * (Ensure your ESP32 can provide sufficient 5V. If not, consider a separate 5V external power supply with common ground).
* **Relay Module `IN1`** --connects to--\> **ESP32 GPIO 25 (`PUMP_NUTRISI_A_PIN`)**.
* **Relay Module `IN2`** --connects to--\> **ESP32 GPIO 26 (`PUMP_NUTRISI_B_PIN`)**.
* **Relay Module `IN3`** --connects to--\> **ESP32 GPIO 27 (`PUMP_PH_PIN`)**.

**B. Load Side (12V Power Supply to Relay to Pump):**

* **For Each Pump (Nutrient A, B, pH):**
    * **Yellow Wire (DC +12V)** from your 12V Power Supply --connects to--\> **`COM`** terminal on the relay channel you are using (e.g., `COM1`, `COM2`, `COM3`).
    * **`NO` (Normally Open)** terminal on the relay channel you are using (e.g., `NO1`, `NO2`, `NO3`) --connects to--\> **Positive (+) Wire of your Pump**.
    * **Negative (-) Wire of your Pump** --connects directly to--\> **Blue Wire (DC -V / GND) of your 12V Power Supply**.

**C. Jumper Settings on the Relay Module:**

* **Crucial:** On your relay module, move the `Low - Com - High` jumper to connect the **`Com` pin with the `High` pin** for each channel you are using (e.g., channels 1, 2, and 3). This setting makes the relay a "HIGH Level Trigger", which matches your `main.cpp` code.

## Software Preparation

### Arduino IDE

1.  **Install ESP32 Board Manager:**

    * Open Arduino IDE.
    * Go to `File > Preferences`.
    * In "Additional Boards Manager URLs", add the following URL:
      `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
    * Go to `Tools > Board > Boards Manager...`.
    * Search for and install "esp32 by Espressif Systems".

2.  **Install Libraries:**

    * Go to `Sketch > Include Library > Manage Libraries...`.
    * Search for and install the following libraries:
        * `PubSubClient by Nick O'Leary`
        * `NewPing by Tim Eckel`
        * `OneWire by Paul Stoffregen`
        * `DallasTemperature by Miles Burton`
        * `Adafruit Unified Sensor by Adafruit`
        * `DHT sensor library by Adafruit`

3.  **Configure `main.cpp` Code:**

    * Open the `main.cpp` file provided in this repository.
    * Adjust your Wi-Fi configuration:
      ```cpp
      const char *WIFI_SSID = "Your_WiFi_Name";
      const char *WIFI_PASSWORD = "Your_WiFi_Password";
      ```
    * Adjust your MQTT Broker configuration:
      ```cpp
      const char *MQTT_SERVER = "your_mqtt_broker_address"; // e.g., broker.hivemq.com or your Home Assistant IP
      const int MQTT_PORT = 1883;
      const char *MQTT_USERNAME = "your_mqtt_username";
      const char *MQTT_PASSWORD = "your_mqtt_password";
      ```
    * Adjust `MQTT_CLIENT_ID` (must be unique for each device).
    * Adjust `TANDON_MAX_HEIGHT_CM` and `WATER_LEVEL_CRITICAL_CM` according to your reservoir size.
    * Ensure pump pin definitions (`PUMP_NUTRISI_A_PIN`, `PUMP_NUTRISI_B_PIN`, `PUMP_PH_PIN`) match the ESP32 GPIO pins connected to your relay module.
    * Save changes.

4.  **Upload Code:**

    * Select your ESP32 board from `Tools > Board > ESP32 Arduino`.
    * Select the correct Serial port from `Tools > Port`.
    * Upload the code to your ESP32.

### Home Assistant

1.  **MQTT Broker Configuration:**

    * Ensure you have an MQTT Broker (e.g., Mosquitto Broker Add-on) installed and configured in your Home Assistant.

2.  **MQTT Switch Configuration:**

    * You will need a configured MQTT Switch file. Based on our previous discussion, your configuration will be under the `mqtt:` block in your `configuration.yaml` Home Assistant file.
    * Example `configuration.yaml`:
      ```yaml
      # Example configuration.yaml
      mqtt:
        broker: YOUR_HA_IP_ADDRESS_OR_BROKER_DOMAIN
        username: YOUR_MQTT_USERNAME
        password: YOUR_MQTT_PASSWORD
        client_id: homeassistant_client_id # A different ID from the ESP32
        discovery: true # Optional, if you use MQTT Discovery
        # ... other MQTT configurations if any
        switch: !include mqtt_switches.yaml # Includes the MQTT switch configuration file
      ```
    * Create an `mqtt_switches.yaml` file (in your `config` or `packages` folder) with the following example content:
      ```yaml
      # mqtt_switches.yaml
      - unique_id: pump_nutrisi_a_hidro
        name: "Nutrient Pump A"
        state_topic: "hidroponik/pompa/nutrisi_a/status"
        command_topic: "hidroponik/pompa/nutrisi_a/kontrol"
        payload_on: "ON"
        payload_off: "OFF"
        optimistic: false
        qos: 0
        retain: true

      - unique_id: pump_nutrisi_b_hidro
        name: "Nutrient Pump B"
        state_topic: "hidroponik/pompa/nutrisi_b/status"
        command_topic: "hidroponik/pompa/nutrisi_b/kontrol"
        payload_on: "ON"
        payload_off: "OFF"
        optimistic: false
        qos: 0
        retain: true

      - unique_id: pump_ph_hidro
        name: "pH Pump"
        state_topic: "hidroponik/pompa/ph/status"
        command_topic: "hidroponik/pompa/ph/kontrol"
        payload_on: "ON"
        payload_off: "OFF"
        optimistic: false
        qos: 0
        retain: true
      ```

3.  **MQTT Sensor Configuration:**

    * Add the sensor configuration to your `configuration.yaml` or an included `mqtt_sensors.yaml` file:
      ```yaml
      # mqtt_sensors.yaml
      - platform: mqtt
        name: "Reservoir Water Level"
        state_topic: "hidroponik/air/level_cm"
        unit_of_measurement: "cm"
        device_class: water_level
        value_template: "{{ value | float }}"

      - platform: mqtt
        name: "Water Sensor Distance"
        state_topic: "hidroponik/air/jarak_sensor_cm"
        unit_of_measurement: "cm"
        device_class: distance
        value_template: "{{ value | float }}"

      - platform: mqtt
        name: "Water Temperature"
        state_topic: "hidroponik/air/suhu_c"
        unit_of_measurement: "°C"
        device_class: temperature
        value_template: "{{ value | float }}"

      - platform: mqtt
        name: "Air Temperature"
        state_topic: "hidroponik/udara/suhu_c"
        unit_of_measurement: "°C"
        device_class: temperature
        value_template: "{{ value | float }}"

      - platform: mqtt
        name: "Air Humidity"
        state_topic: "hidroponik/udara/kelembaban_persen"
        unit_of_measurement: "%"
        device_class: humidity
        value_template: "{{ value | float }}"

      - platform: mqtt
        name: "ESP32 Hydroponics Status"
        state_topic: "tele/esp32hidro/LWT"
        value_template: "{{ value }}"
        # device_class: connectivity # Optional, depending on HA version

      - platform: mqtt
        name: "Hydroponics Alert"
        state_topic: "hidroponik/peringatan"
        value_template: "{{ value }}"
        icon: mdi:alert
      ```

4.  **Restart Home Assistant:** After adding all configurations, restart Home Assistant for the changes to take effect.

5.  **Add to Lovelace Dashboard:** After restarting, you can add the switch entities (pump buttons) and sensors to your Lovelace dashboard.

## Assembly & Wiring Instructions

Follow the wiring guide detailed in the [Wiring Diagram](https://www.google.com/search?q=%23wiring-diagram) section. Ensure all connections are secure and properly insulated.

## Usage

Once everything is assembled and configured:

1.  Power on your 12V DC Power Supply.
2.  The ESP32 will attempt to connect to Wi-Fi and then to your MQTT Broker.
3.  Sensor data will begin to publish to Home Assistant every 5 seconds.
4.  You can control the pumps from Home Assistant.
5.  If the reservoir water level reaches a critical threshold (20 cm or less), the system will send an MQTT alert and activate the buzzer.

## Troubleshooting

* **Pumps not turning ON/OFF despite commands being received in Home Assistant:**
    * **Check Wiring Connections:** Ensure all wires are securely and correctly connected, especially `GND`, which must be common to all components (ESP32, Relay, Power Supply).
    * **Check Relay Power:** Ensure the relay module is receiving sufficient and stable 5V power on its `DC+` pin.
    * **Check Relay Jumper Settings:** Ensure the `Low - Com - High` jumper on the relay module is set to the **`Com - High`** position. This ensures the relay functions as a "HIGH Level Trigger" according to your code.
    * **Test Relay Independently:** You can manually test the relay by connecting its `IN` pin to ESP32's `3.3V` or `GND` (with the relay's VCC/GND already connected) and listen for a "click" sound.
* **Sensors not reading data:**
    * Double-check the data pin connections of the sensors to the ESP32.
    * Ensure the sensor libraries are correctly installed in the Arduino IDE.
    * Ensure the sensor power (VCC/GND) is connected correctly.
* **ESP32 not connecting to Wi-Fi/MQTT:**
    * Double-check your `WIFI_SSID`, `WIFI_PASSWORD`, `MQTT_SERVER`, `MQTT_USERNAME`, and `MQTT_PASSWORD` in the code.
    * Ensure the ESP32 is within Wi-Fi range.
    * Ensure your MQTT Broker is running and accessible from the same network.
    * Use the Arduino IDE Serial Monitor to view connection logs.
* **Entities not appearing in Home Assistant:**
    * Double-check your MQTT configuration in `configuration.yaml` and `mqtt_switches.yaml`/`mqtt_sensors.yaml`.
    * Ensure there are no indentation errors in the YAML.
    * Restart Home Assistant after every YAML configuration change.
    * Check Home Assistant logs for any MQTT-related error messages.

## Contribution

Contributions to this project are highly welcome\! If you have suggestions, improvements, or find any bugs, please feel free to open an issue or submit a pull request.

## License

This project is licensed under the [MIT License](https://opensource.org/licenses/MIT). You are free to use, modify, and distribute this code for personal or commercial purposes, with appropriate attribution.