# ESP32 LoRa-to-Internet Gateway: Implementation Documentation

This document describes the actual implementation of the ESP32 LoRa-to-Internet Gateway project, based on the codebase as of March 1st, 2026.

## 1. Core Components

The firmware is divided into the following main components:

- **`main.cpp`**: The main application logic, responsible for initializing all other components and running the main loop.
- **`config`**: Manages the device's configuration, saving and loading settings from a `config.json` file in the device's flash memory.
- **`battery`**: Monitors the battery voltage and converts it to a percentage.
- **`display`**: Controls the OLED display, showing a status dashboard.
- **`lora_radio`**: Manages the LoRa radio, including receiving packets and configuring radio parameters.
- **`wifi_manager`**: Handles WiFi connectivity, including Station (STA) and Access Point (AP) modes, and provides a captive portal.
- **`web_server`**: Implements a web-based configuration portal for in-field setup.
- **`forwarder`**: Forwards received LoRa packets to a server via MQTT or HTTP.

## 2. Functional Breakdown

### 2.1. Configuration (`config.cpp`, `config.h`)

- **Persistence**: All settings are stored in a `config.json` file on the LittleFS partition, which survives reboots.
- **Defaults**: If `config.json` is not found on the first boot, default settings are applied, and the file is created.
- **Global Access**: A global `gConfig` struct provides easy access to all configuration parameters throughout the firmware.
- **Parameters Stored**:
    - WiFi SSID and password
    - LoRa settings (frequency, SF, bandwidth, coding rate, sync word)
    - Forwarding mode (MQTT or HTTP)
    - MQTT broker details (URL, port, topic, user, pass)
    - HTTP endpoint URL
    - Gateway ID

**PRD Comparison**: Fully implements the persistence and configuration requirements.

### 2.2. Captive Portal & Web Server (`web_server.cpp`, `web_pages.h`)

- **Trigger**: The captive portal is automatically triggered if no WiFi SSID is stored in the configuration. The PRD's suggestion of a physical button is not implemented.
- **Access Point**: The device creates an AP named `LoRa-GW-Setup`.
- **Web Interface**: A single, responsive web page is served, allowing the user to configure all device parameters.
    - **Live Data**: The page is dynamically generated on the server-side, pre-filling all fields with their current values.
    - **WiFi Setup**: Includes a list of scanned networks and a manual entry option.
    - **LoRa Settings**: Full control over frequency (with presets for EU868), SF, bandwidth, and coding rate.
    - **Forwarding**: A toggle between MQTT and HTTP, with conditional visibility for the relevant settings.
- **Saving**: When a form is submitted, the new settings are saved to `config.json`, and the relevant components are updated in the main loop.

**PRD Comparison**: Exceeds the PRD requirements by providing a more comprehensive and user-friendly web interface.

### 2.3. Battery Monitoring (`battery.cpp`, `battery.h`)

- **Sensing**: Reads the voltage from the internal voltage divider connected to **GPIO 35**.
- **Voltage-to-Percentage**: The raw ADC value is converted to a voltage, which is then mapped to a 0-100% value using a piecewise linear interpolation based on a LiPo discharge curve.
- **Safety Feature**: A `batteryIsCritical()` function returns `true` if the battery level is below 5%. In the main loop, this is used to disable the LoRa radio to prevent deep discharge.

**PRD Comparison**: Fully implements the battery monitoring and safety requirements.

### 2.4. Data Forwarding (`forwarder.cpp`, `forwarder.h`)

- **Dual Protocol**: Supports forwarding to either an MQTT broker or an HTTP endpoint, as selected in the web interface.
- **JSON Payload**: All forwarded LoRa packets are converted into a JSON object with the following fields:
    - `gateway_id`
    - `timestamp`
    - `rssi`
    - `snr`
    - `spreading_factor`
    - `frequency`
    - `packet_size`
    - `raw_payload` (as a hex string)
- **Libraries**: Uses `PubSubClient` for MQTT and `HTTPClient` for HTTP.

**PRD Comparison**: Fully implements the data forwarding and payload construction requirements.

### 2.5. User Interface (`display.cpp`, `display.h`)

- **OLED Dashboard**: The 128x64 OLED display shows a dashboard with the following information:
    - **Header**: WiFi status icon, IP address (or "AP Mode"), and battery percentage with a visual icon.
    - **Activity**: Information on the last received packet (SF/RSSI) or a "Waiting..." message.
    - **Counters**: Total number of packets received and forwarded since boot.
    - **LoRa Status**: Shows if the LoRa radio is disabled due to a low battery.

**PRD Comparison**: Fully implements the OLED display requirements.

## 3. Deviations and Unimplemented Features

- **Physical "Config" Button**: The PRD mentions a physical button to force the device into AP mode. This has not been implemented. The AP mode is only triggered automatically when no WiFi configuration is saved. This is a minor deviation, as the primary trigger mechanism is functional.
