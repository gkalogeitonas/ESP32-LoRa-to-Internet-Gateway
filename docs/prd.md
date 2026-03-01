## PRD: ESP32 LoRa-to-Internet Gateway

**Device:** TTGO LoRa32 v2.1 (868MHz)

**Target:** Single-Channel Packet Forwarding

### 1. Product Overview

A specialized IoT gateway that bridges LoRa radio packets to a centralized server. It features a local configuration portal (Captive Portal) for field deployment without needing to re-flash the firmware for different networks.

### 2. Functional Requirements

#### 2.1 Captive Portal & Configuration

* **Trigger:** If no WiFi credentials are saved, or a physical "Config" button is held for 5 seconds, the device spawns an AP (e.g., `LoRa-GW-Setup`).
* **Web Interface Features:**
* **WiFi Setup:** SSID scanning and password entry.
* **LoRa Settings:** Frequency (868MHz base), Spreading Factor, Bandwidth.
* **Forwarding Selection:** A toggle to choose **either** MQTT **or** HTTP.
* **MQTT Params:** Broker URL, Port, Topic, Auth (User/Pass).
* **HTTP Params:** Endpoint URL (POST).



#### 2.2 Battery Monitoring System

* **Hardware Sensing:** Read voltage via the internal divider on **GPIO 35**.
* **Logic (Voltage to %):** Convert the raw ADC value to a percentage based on a standard LiPo discharge curve:
* **Display:** The OLED must show a percentage integer ($0-100\%$) alongside a visual battery icon.

#### 2.3 Data Forwarding (Single-Path)

* **Payload Construction:** Every incoming LoRa packet is converted to a JSON object containing:



* **Protocol Execution:** * If **MQTT** is selected: Publish to the configured topic.
* If **HTTP** is selected: Send via an asynchronous HTTP POST request.



### 3. User Interface (OLED 128x64)

The screen is the primary diagnostic tool. It should display a "Dashboard" view:

| Section | Content |
| --- | --- |
| **Header** | WiFi Status Icon | **Battery % (e.g., 85%)** |
| **Network** | Current IP Address (or "AP Mode Active") |
| **Activity** | Last Received Packet (SF / RSSI) |
| **Counters** | Total Packets forwarded since boot |

### 4. Technical Specifications

| Feature | Specification |
| --- | --- |
| **MCU** | ESP32-D0WDQ6 (Dual Core) |
| **Radio** | SX1276 (868MHz) |
| **Display** | SSD1306 (I2C) |
| **Storage** | LittleFS (for `config.json` and Web UI assets) |

---

### 5. Non-Functional Requirements

* **Reliability:** The gateway must automatically attempt to reconnect to WiFi every 60 seconds if the connection is lost.
* **Persistence:** All settings must survive a power cycle (saved in Flash memory).
* **Safety:** The device should cease LoRa operations if the battery hits $5\%$ to prevent deep-discharge damage to the 18650 cell.

---
