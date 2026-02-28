#include "lora_radio.h"
#include "pins.h"
#include <SPI.h>
#include <LoRa.h>

static bool _loraEnabled = false;

bool loraInit(const GatewayConfig &cfg) {
    SPI.begin(LORA_SCK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN, LORA_SS_PIN);
    LoRa.setPins(LORA_SS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);

    if (!LoRa.begin(cfg.lora_frequency)) {
        Serial.println("[LoRa] Init failed");
        return false;
    }

    loraApplySettings(cfg);

    LoRa.receive();
    _loraEnabled = true;

    Serial.printf("[LoRa] Init OK: freq=%ld SF=%d BW=%ld\n",
                  cfg.lora_frequency, cfg.lora_sf, cfg.lora_bandwidth);
    return true;
}

void loraApplySettings(const GatewayConfig &cfg) {
    LoRa.setSpreadingFactor(cfg.lora_sf);
    LoRa.setSignalBandwidth(cfg.lora_bandwidth);
    LoRa.setCodingRate4(cfg.lora_coding_rate);
    LoRa.setSyncWord(cfg.lora_sync_word);

    if (_loraEnabled) {
        LoRa.receive();
    }

    Serial.printf("[LoRa] Settings applied: SF=%d BW=%ld CR=4/%d\n",
                  cfg.lora_sf, cfg.lora_bandwidth, cfg.lora_coding_rate);
}

bool loraPoll(LoRaPacket &pkt) {
    int packetSize = LoRa.parsePacket();
    if (packetSize == 0) return false;

    pkt.size = 0;
    while (LoRa.available() && pkt.size < (int)sizeof(pkt.raw)) {
        pkt.raw[pkt.size++] = LoRa.read();
    }

    pkt.rssi      = LoRa.packetRssi();
    pkt.snr       = LoRa.packetSnr();
    pkt.frequency = gConfig.lora_frequency;
    pkt.sf        = gConfig.lora_sf;
    pkt.timestamp = millis();

    return true;
}

void loraEnable() {
    LoRa.receive();
    _loraEnabled = true;
    Serial.println("[LoRa] Enabled");
}

void loraDisable() {
    LoRa.sleep();
    _loraEnabled = false;
    Serial.println("[LoRa] Disabled (sleep)");
}

bool loraIsEnabled() {
    return _loraEnabled;
}
