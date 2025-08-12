# ESP32 Hydroponics Automation System Project

This project is an automation system for hydroponic cultivation using an ESP32, MQTT, and integration with Home Assistant. The system monitors important parameters such as water level, water temperature, air temperature, humidity, and Total Dissolved Solids (TDS). Furthermore, it provides precise, volume-based control for nutrient and pH pumps, along with an early warning system for critical water levels.

## Table of Contents

- [ESP32 Hydroponics Automation System Project](#esp32-hydroponics-automation-system-project)
  - [Table of Contents](#table-of-contents)
  - [Main Features](#main-features)
  - [Component List](#component-list)
  - [System Architecture (Two-Box Setup)](#system-architecture-two-box-setup)
    - [Inter-Box Connection (LAN Cable)](#inter-box-connection-lan-cable)
  - [Wiring Diagram](#wiring-diagram)
    - [ESP32 Pinout](#esp32-pinout)
    - [DC 12V 5A Power Supply Connections](#dc-12v-5a-power-supply-connections)
    - [8-Channel Relay Module Connections](#8-channel-relay-module-connections)
  - [Software Preparation (PlatformIO)](#software-preparation-platformio)
  - [Home Assistant Configuration](#home-assistant-configuration)
  - [Usage](#usage)
  - [Troubleshooting](#troubleshooting)
  - [Contribution](#contribution)
  - [License](#license)

## Main Features

*   **Comprehensive Monitoring:**
    *   **Water:** Level (Ultrasonic), Temperature (DS18B20), TDS (with temperature compensation), and pH (with 4-point calibration).
    *   **Environment:** Air Temperature & Humidity (DHT22).
    *   **Electrical:** Voltage, Current, Power, Energy, Frequency, and Power Factor (PZEM-004T).
*   **Precise Manual Control:**
    *   **Dosing:** Control Nutrient A, B, and pH pumps by volume (ml).
    *   **Watering:** Control the watering pump by duration (seconds).
    *   **Reservoir Refill:** Manual ON/OFF control for the refill valve.
*   **Full Automation Suite:**
    *   **Auto-Dosing:** Automatically maintains TDS and pH levels based on configurable targets.
    *   **Auto-Refill:** Automatically refills the reservoir when the water level is low and stops when full.
    *   **Smart Watering:** Performs scheduled (hourly) and temperature-based watering.
*   **Advanced Home Assistant Integration:**
    *   All sensor data and actuator controls are fully integrated via MQTT.
    *   Custom dashboard with 3 tabs: Monitoring, Manual Controls, and Settings.
    *   Two-way synchronization between automation statuses in the UI and on the ESP32 device.
*   **Reliability & Security:**
    *   Automatic Wi-Fi and MQTT reconnection.
    *   MQTT Last Will and Testament (LWT) for accurate online/offline status.
    *   Critical water level alerts with a buzzer and MQTT notifications.
    *   Secure credential management using a separate `credentials.ini` file.

## Component List

| Component                     | Qty | Description                                        |
| :---------------------------- | :-: | :------------------------------------------------- |
| ESP32 Development Board       | 1   | Recommended: NodeMCU ESP32, ESP32 DevKitC          |
| DC 12V Power Supply           | 1   | For powering pumps and the relay module            |
| DC 12V Pump                   | 4   | 3x Peristaltic (Nutrients/pH), 1x Watering Pump    |
| Relay Module                  | 1   | Recommended: 4-channel or 8-channel              |
| JSN-SR04T Ultrasonic Sensor   | 1   | For water level measurement                        |
| DS18B20 Temperature Sensor    | 1   | For water temperature                              |
| DHT22 Temp & Humidity Sensor  | 1   | For air temperature & humidity                     |
| Analog TDS Meter              | 1   | For measuring water nutrient concentration         |
| PH-4502C pH Sensor            | 1   | For measuring water pH level                       |
| PZEM-004T v4 Power Sensor     | 1   | For measuring power consumption                    |
| Active Buzzer                 | 1   | For audible alerts                                 |
| Jumper Wires                  | -   | Male-to-Male, Female-to-Female, Male-to-Female     |
| Enclosure Box (optional)      | 2   | One for controller, one for wet sensors            |

## System Architecture (Two-Box Setup)

To enhance safety and modularity, this system is designed to be split into two separate enclosures:

*   **Box 1 (Controller Box):** Houses the main processing and high-voltage components, kept in a dry area.
    *   ESP32 Development Board
    *   PZEM-004T Power Sensor
    *   Relay Module
    *   Buzzer
    *   DHT22 Air Temp & Humidity Sensor

*   **Box 2 (Sensor Box):** Houses all the "wet" sensors that are placed near or in the water reservoir.
    *   JSN-SR04T Ultrasonic Sensor
    *   DS18B20 Water Temperature Sensor
    *   Analog TDS Sensor
    *   PH-4502C pH Sensor

### Inter-Box Connection (LAN Cable)

A single **Cat5e or Cat6 LAN cable** (up to 2-3 meters) is used to connect the two boxes. This provides a neat and organized way to run all the necessary wires.

| Signal Function        | ESP32 Pin (Box 1) | LAN Cable Wire Color (T568B) | Notes                                    |
| :--------------------- | :---------------- | :--------------------------- | :--------------------------------------- |
| **5V Power**           | `5V`              | Brown                        | Power for Ultrasonic Sensor              |
| **Ground**             | `GND`             | White-Brown                  | **Common Ground** for all sensors        |
| **3.3V Power**         | `3V3`             | Blue                         | Power for pH, TDS, and DS18B20 sensors   |
| **Water Temp Data**    | `GPIO 18`         | White-Blue                   | DS18B20 One-Wire data signal             |
| **Ultrasonic Trigger** | `GPIO 5`          | Orange                       | Ultrasonic trigger signal                |
| **Ultrasonic Echo**    | `GPIO 4`          | White-Orange                 | Ultrasonic echo signal                   |
| **pH Data**            | `GPIO 32`         | Green                        | Analog signal from pH sensor             |
| **TDS Data**           | `GPIO 34`         | White-Green                  | Analog signal from TDS sensor            |

**⚠️ Important Warning:** Extending sensor wires, especially for analog (pH, TDS) and timing-sensitive digital (Ultrasonic, DS18B20) signals, over a distance of 2-3 meters can introduce signal noise and degradation. This can lead to inaccurate or unstable readings. While this setup can work, a more robust professional solution would involve placing a secondary microcontroller (like an ESP8266) in Box 2 to process sensor data locally and transmit it digitally. Proceed with this single-controller setup at your own discretion.

## Wiring Diagram

Here are the detailed pin connections between the ESP32, Sensors, Buzzer, Power Supply, Pumps, and Relay Module.

**Important:** Ensure all `GND` connections are common (`Common Ground`). This means the ESP32's `GND`, the relay module's `DC -`, and the 12V Power Supply's `-V` must be connected to the same point.

### ESP32 Pinout

| ESP32 Pin (GPIO) | Connected Component             | Description                                     |
| :--------------- | :------------------------------ | :---------------------------------------------- |
| 5                | ULTRASONIC_TRIGGER_PIN (JSN-SR04T Trig) | Ultrasonic Sensor Trigger Signal                |
| 4                | ULTRASONIC_ECHO_PIN (JSN-SR04T Echo)   | Ultrasonic Sensor Echo Signal                   |
| 18               | ONE_WIRE_BUS (DS18B20 Data)            | Water Temperature Sensor Data Pin               |
| 13               | DHT_PIN (DHT22 Data)                   | Air Temperature & Humidity Sensor Data Pin      |
| 19               | BUZZER_PIN                      | Buzzer Control Pin                              |
| 34               | TDS_SENSOR_PIN                 | Analog signal from TDS sensor adapter board     |
| 32               | PH_SENSOR_PIN                  | Analog signal from pH sensor adapter board      |
| 16               | PZEM-004T TX (to ESP32 RX2)     | Power Sensor Data Out                           |
| 17               | PZEM-004T RX (to ESP32 TX2)     | Power Sensor Data In                            |
| 25               | PUMP_NUTRISI_A_PIN (Relay IN1)  | Nutrient Pump A Control                         |
| 26               | PUMP_NUTRISI_B_PIN (Relay IN2)  | Nutrient Pump B Control                         |
| 27               | PUMP_PH_PIN (Relay IN3)         | pH Pump Control                                 |
| 33               | PUMP_SIRAM_PIN (Relay IN4)      | Watering Pump Control                           |
| 15               | PUMP_TANDON_PIN (Relay IN5)     | Reservoir Refill Valve/Pump Control             |
| GND              | All Component GNDs              | System Common Ground                            |
| 5V (or VIN)      | Relay Module DC+ & PZEM-004T    | Power for Control Circuits                      |
| 3V3              | pH & TDS Module VCC             | Power for Analog Sensor Modules                 |

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
*   **Relay Module `IN4`** --connects to--> **ESP32 GPIO 33 (`PUMP_SIRAM_PIN`)**.
*   **Relay Module `IN5`** --connects to--> **ESP32 GPIO 15 (`PUMP_TANDON_PIN`)**.

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
        mqtt_user = "your_mqtt_username"
        mqtt_pass = "your_mqtt_password"
        mqtt_server = "your_mqtt_broker_ip_or_domain"
        mqtt_port = 1883
        ```

4.  **Upload Firmware:**
    *   Connect your ESP32 board to your computer.
    *   Click the **Upload** button (right-arrow icon) in the PlatformIO status bar at the bottom of the VS Code window. PlatformIO will compile and upload the firmware to your device.

## Home Assistant Configuration

1.  **MQTT Broker Configuration:**
    *   Ensure you have an MQTT Broker (e.g., Mosquitto Broker Add-on) installed and configured in your Home Assistant.

2.  **Entity Configuration in Home Assistant:**
    *   Copy all files and directories from the `src/ha_config/` directory of this project into your Home Assistant configuration directory, preserving the file structure.
    *   Alternatively, use the `Makefile` provided to deploy the configuration automatically.

## Usage

Once everything is assembled and configured:

1.  Power on your 12V DC Power Supply.
2.  The ESP32 will attempt to connect to Wi-Fi and then to your MQTT Broker.
3.  Sensor data will begin to publish to Home Assistant.
4.  You can control the pumps via the Home Assistant dashboard.
5.  To perform an emergency stop on a running pump, send the payload `OFF` to its control topic.
6.  If the reservoir water level reaches a critical threshold, the system will send an MQTT alert and activate the buzzer.

## Troubleshooting

*   **Pumps not activating after sending a volume command:**
    *   **Check Wiring Connections:** Ensure all wires are securely and correctly connected, especially `GND`, which must be common to all components (ESP32, Relay, Power Supply).
    *   **Check Relay Power:** Ensure the relay module is receiving sufficient and stable 5V power on its `DC+` pin.
    *   **Check Relay Jumper Settings:** Ensure the `Low - Com - High` jumper on the relay module is set to the **`Com - High`** position. This ensures the relay functions as a "HIGH Level Trigger" according to your code.
    *   **Check Serial Monitor:** When you send a command, the ESP32 serial monitor should print a `[Pump Control] Pumping X ml...` message. If it doesn't, the MQTT message is not being received or parsed correctly.
*   **Sensors not reading data:**
    *   Double-check the data pin connections of the sensors to the ESP32.
    *   Ensure the relevant sensor libraries have been correctly installed by PlatformIO.
*   **ESP32 not connecting to Wi-Fi/MQTT:**
    *   Use the Serial Monitor in PlatformIO to view detailed connection logs.
    *   Double-check your credentials in the `credentials.ini` file.
    *   Ensure the ESP32 is within Wi-Fi range and your MQTT Broker is accessible.
*   **UI changes not appearing:** Clear your browser cache (Ctrl+F5 or Cmd+Shift+R) and restart Home Assistant after deploying YAML configuration changes.

## Contribution

Contributions to this project are highly welcome! If you have suggestions, improvements, or find any bugs, please feel free to open an issue or submit a pull request.

## License

This project is licensed under the MIT License. You are free to use, modify, and distribute this code for personal or commercial purposes, with appropriate attribution.
