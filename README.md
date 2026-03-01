# ESP32 LoRa-to-Internet Gateway

This project turns a TTGO LoRa32 v2.1 board into a single-channel LoRa packet forwarder. It receives LoRa packets and forwards them to a server over the internet using either MQTT or HTTP.

The main feature is its web-based configuration portal, which allows you to set up the device in the field without needing to re-flash the firmware.

## Features

- **LoRa Packet Forwarding**: Receives LoRa packets on a configurable channel and forwards them.
- **Dual Forwarding Modes**: Forward packets via either MQTT or HTTP POST requests.
- **Web-Based Configuration**: If not connected to a WiFi network, the device starts an Access Point (`LoRa-GW-Setup`). You can connect to this AP and configure the device through a web portal.
- **OLED Display**: A 128x64 OLED screen shows the device's status, including WiFi connection, IP address, battery level, and packet statistics.
- **Battery Monitoring**: Reads the voltage of the connected LiPo battery and displays it as a percentage. To protect the battery, the LoRa radio is disabled if the charge drops below 5%.
- **Persistent Configuration**: All settings are saved in the device's flash memory and survive reboots.

## Getting Started

### Prerequisites

1.  **Hardware**: A TTGO LoRa32 v2.1 (868MHz version) board.
2.  **Software**:
    - [Visual Studio Code](https://code.visualstudio.com/) with the [PlatformIO IDE extension](https://platformio.org/platformio-ide).
    - All library dependencies are listed in the `platformio.ini` file and will be installed automatically by PlatformIO.

### First-Time Setup

1.  **Build and Upload**:
    - Open the project folder in Visual Studio Code.
    - PlatformIO will automatically detect the `platformio.ini` file and download the required libraries.
    - Build and upload the firmware to your TTGO LoRa32 board.

2.  **Configure the Gateway**:
    - On the first boot, the device won't have any WiFi credentials, so it will start an Access Point named **`LoRa-GW-Setup`**.
    - Connect your phone or computer to this WiFi network.
    - Your device should automatically open a captive portal page. If not, open a web browser and navigate to `http://192.168.4.1`.
    - Use the web interface to:
        - Scan for and select your local WiFi network and enter the password.
        - Configure your LoRa settings (the defaults are a good starting point).
        - Choose your forwarding method (MQTT or HTTP) and fill in the server details.
    - Save your settings. The device will then restart and connect to your chosen WiFi network.

## OLED Display

The OLED display provides a quick overview of the gateway's status:

| Section      | Content                                     |
|--------------|---------------------------------------------|
| **Header**   | WiFi Status, IP Address, and Battery %      |
| **Activity** | Details of the last received packet (SF/RSSI) |
| **Counters** | Total packets received and forwarded        |

## Implementation Details

For a detailed breakdown of the firmware's implementation and how it compares to the original product requirements, please see the [Implementation Documentation](implementation_docs.md).
