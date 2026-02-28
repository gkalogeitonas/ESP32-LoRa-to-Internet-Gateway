#ifndef LORA_RADIO_H
#define LORA_RADIO_H

#include <Arduino.h>
#include "config.h"

struct LoRaPacket {
    uint8_t raw[256];
    int     size;
    int     rssi;
    float   snr;
    long    frequency;
    int     sf;
    unsigned long timestamp;
};

bool loraInit(const GatewayConfig &cfg);
void loraApplySettings(const GatewayConfig &cfg);
bool loraPoll(LoRaPacket &pkt);
void loraEnable();
void loraDisable();
bool loraIsEnabled();

#endif
