#include "display.h"
#include "pins.h"
#include <Wire.h>
#include "SSD1306Wire.h"

static SSD1306Wire oled(OLED_ADDR, OLED_SDA_PIN, OLED_SCL_PIN);

// WiFi icon 8x8 XBM
static const uint8_t wifiIcon[] PROGMEM = {
    0x00, 0x3C, 0x42, 0x18, 0x24, 0x00, 0x08, 0x00
};

// No-WiFi icon 8x8 XBM
static const uint8_t noWifiIcon[] PROGMEM = {
    0x81, 0x7E, 0x66, 0x18, 0x18, 0x66, 0x7E, 0x81
};

static void drawBatteryIcon(int x, int y, int percent) {
    // Battery outline: 20x10 with terminal nub
    oled.drawRect(x, y, 20, 10);
    oled.fillRect(x + 20, y + 2, 2, 6);
    // Fill level
    int fillWidth = map(constrain(percent, 0, 100), 0, 100, 0, 18);
    if (fillWidth > 0) {
        oled.fillRect(x + 1, y + 1, fillWidth, 8);
    }
}

void displayInit() {
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
    delay(100);

    oled.init();
    oled.flipScreenVertically();
    oled.setFont(ArialMT_Plain_10);
    oled.setTextAlignment(TEXT_ALIGN_LEFT);

    oled.clear();
    oled.drawString(0, 0, "LoRa Gateway");
    oled.drawString(0, 16, "Starting...");
    oled.display();

    Serial.println("[Display] OLED initialized");
}

void displayShowMessage(const char* line1, const char* line2) {
    oled.clear();
    oled.setFont(ArialMT_Plain_16);
    oled.setTextAlignment(TEXT_ALIGN_CENTER);
    oled.drawString(64, 10, line1);
    if (line2) {
        oled.drawString(64, 32, line2);
    }
    oled.display();
    oled.setFont(ArialMT_Plain_10);
    oled.setTextAlignment(TEXT_ALIGN_LEFT);
}

void displayUpdate(
    bool wifiConnected,
    bool apMode,
    const String &ipAddr,
    int batteryPercent,
    int lastRssi,
    int lastSf,
    unsigned long totalPackets,
    unsigned long totalPacketsRx,
    bool loraEnabled,
    long loraFrequency,
    int loraConfigSf
) {
    oled.clear();

    // --- Header row: WiFi icon + IP + Battery ---
    if (wifiConnected || apMode) {
        oled.drawXbm(0, 0, 8, 8, wifiIcon);
    } else {
        oled.drawXbm(0, 0, 8, 8, noWifiIcon);
    }

    // Battery percentage text + icon
    String battStr = String(batteryPercent) + "%";
    oled.drawString(104 - oled.getStringWidth(battStr), 0, battStr);
    drawBatteryIcon(104, 0, batteryPercent);

    // --- Network line ---
    if (apMode) {
        oled.drawString(12, 0, "AP Mode");
    } else if (wifiConnected) {
        oled.drawString(12, 0, ipAddr);
    } else {
        oled.drawString(12, 0, "No WiFi");
    }

    // Separator
    oled.drawHorizontalLine(0, 13, 128);

    // --- Activity: last received packet ---
    if (!loraEnabled) {
        oled.drawString(0, 16, "LoRa OFF (low batt)");
    } else if (lastRssi == 0 && lastSf == 0) {
        oled.drawString(0, 16, "Waiting for packets...");
    } else {
        oled.drawString(0, 16, "Last: SF" + String(lastSf) + " / " + String(lastRssi) + " dBm");
    }

    // --- Configured channel + SF ---
    char chBuf[28];
    float freqMHz = loraFrequency / 1000000.0f;
    snprintf(chBuf, sizeof(chBuf), "Ch: %.1f MHz  SF:%d", freqMHz, loraConfigSf);
    oled.drawString(0, 28, chBuf);

    // --- Counters ---
    oled.drawHorizontalLine(0, 40, 128);
    char cntBuf[28];
    snprintf(cntBuf, sizeof(cntBuf), "Rx:%lu  Fwd:%lu", totalPacketsRx, totalPackets);
    oled.drawString(0, 44, cntBuf);

    // Forwarding mode indicator (right-aligned)
    oled.setTextAlignment(TEXT_ALIGN_RIGHT);
    oled.drawString(128, 44, apMode ? "AP" : (wifiConnected ? "STA" : "---"));
    oled.setTextAlignment(TEXT_ALIGN_LEFT);

    oled.display();
}
