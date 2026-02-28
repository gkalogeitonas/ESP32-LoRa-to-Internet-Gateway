#include <Arduino.h>
#include "pins.h"
#include "config.h"
#include "battery.h"
#include "display.h"
#include "lora_radio.h"
#include "wifi_manager.h"
#include "web_server.h"
#include "forwarder.h"

// Timing intervals (milliseconds)
#define BATTERY_CHECK_INTERVAL   30000
#define DISPLAY_UPDATE_INTERVAL   1000
#define MQTT_LOOP_INTERVAL         100

// State tracking (some are extern'd by web_server.cpp for /status)
unsigned long totalPacketsForwarded = 0;
int  currentBatteryPercent = 100;

static unsigned long lastBatteryCheck  = 0;
static unsigned long lastDisplayUpdate = 0;
static unsigned long lastMqttLoop      = 0;
static bool loraActive = true;
static int  lastPacketRssi = 0;
static int  lastPacketSf   = 0;

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("================================");
    Serial.println(" LoRa-to-Internet Gateway");
    Serial.println("================================");

    // LED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // 1. Configuration (must be first)
    if (!configInit()) {
        Serial.println("FATAL: Config init failed");
    }

    // 2. Display (splash during init)
    displayInit();
    displayShowMessage("LoRa Gateway", "Starting...");

    // 3. Battery
    batteryInit();
    currentBatteryPercent = batteryReadPercent();
    Serial.printf("[Boot] Battery: %d%% (%.2fV)\n",
                  currentBatteryPercent, batteryReadVoltage());

    // 4. LoRa radio
    if (!loraInit(gConfig)) {
        displayShowMessage("LoRa FAILED!", "Check hardware");
        Serial.println("FATAL: LoRa init failed");
        while (1) { delay(1000); }
    }

    // 5. WiFi (STA or AP)
    wifiInit(gConfig);

    // 6. Scan WiFi networks (synchronous, before web server starts)
    displayShowMessage("LoRa Gateway", "Scanning WiFi...");
    wifiScanSync();

    // 7. Web server (async, no blocking)
    webServerInit();

    // 8. Forwarder (MQTT or HTTP)
    forwarderInit(gConfig);

    displayShowMessage("LoRa Gateway", "Ready!");
    delay(1000);

    Serial.println("[Boot] Setup complete");
}

void loop() {
    unsigned long now = millis();

    // --- Handle config change flags from web server ---
    if (flagWifiChanged) {
        flagWifiChanged = false;
        Serial.println("[Main] WiFi config changed, reconnecting...");
        wifiInit(gConfig);
    }
    if (flagLoraChanged) {
        flagLoraChanged = false;
        Serial.println("[Main] LoRa config changed, applying...");
        loraApplySettings(gConfig);
    }
    if (flagForwardChanged) {
        flagForwardChanged = false;
        Serial.println("[Main] Forwarding config changed, updating...");
        forwarderUpdateConfig(gConfig);
    }
    if (flagRestartRequested) {
        displayShowMessage("Restarting...", "");
        delay(1000);
        ESP.restart();
    }
    if (flagRescanRequested) {
        flagRescanRequested = false;
        Serial.println("[Main] WiFi rescan requested");
        wifiScanSync();
    }

    // --- 1. Battery check (every 30s) ---
    if (now - lastBatteryCheck >= BATTERY_CHECK_INTERVAL) {
        lastBatteryCheck = now;
        currentBatteryPercent = batteryReadPercent();

        if (batteryIsCritical() && loraActive) {
            loraDisable();
            loraActive = false;
            Serial.println("WARNING: Battery critical (<5%). LoRa disabled.");
        }
    }

    // --- 2. WiFi management ---
    wifiLoop();

    // --- 3. LoRa packet polling ---
    if (loraActive) {
        LoRaPacket pkt;
        if (loraPoll(pkt)) {
            lastPacketRssi = pkt.rssi;
            lastPacketSf   = pkt.sf;

            digitalWrite(LED_PIN, HIGH);

            Serial.printf("[LoRa] Packet: %d bytes RSSI=%d SNR=%.1f\n",
                          pkt.size, pkt.rssi, pkt.snr);

            // Forward if WiFi is connected
            if (wifiIsConnected()) {
                if (forwarderSend(pkt, gConfig)) {
                    totalPacketsForwarded++;
                }
            } else {
                Serial.println("[LoRa] No WiFi, packet not forwarded");
            }

            digitalWrite(LED_PIN, LOW);
        }
    }

    // --- 4. MQTT keepalive ---
    if (gConfig.use_mqtt && wifiIsConnected() && (now - lastMqttLoop >= MQTT_LOOP_INTERVAL)) {
        lastMqttLoop = now;
        forwarderLoop();
    }

    // --- 5. Display update (every 1s) ---
    if (now - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
        lastDisplayUpdate = now;
        displayUpdate(
            wifiIsConnected(),
            wifiIsAPMode(),
            wifiGetIP(),
            currentBatteryPercent,
            lastPacketRssi,
            lastPacketSf,
            totalPacketsForwarded,
            loraActive
        );
    }
}
