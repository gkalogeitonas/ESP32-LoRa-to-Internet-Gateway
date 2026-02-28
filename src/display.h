#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

void displayInit();
void displayUpdate(
    bool wifiConnected,
    bool apMode,
    const String &ipAddr,
    int batteryPercent,
    int lastRssi,
    int lastSf,
    unsigned long totalPackets,
    bool loraEnabled
);
void displayShowMessage(const char* line1, const char* line2 = nullptr);

#endif
