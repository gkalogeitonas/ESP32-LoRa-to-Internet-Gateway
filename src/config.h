#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

struct GatewayConfig {
    // WiFi
    char wifi_ssid[33];
    char wifi_pass[65];

    // LoRa
    long lora_frequency;
    int  lora_sf;
    long lora_bandwidth;
    int  lora_coding_rate;
    uint8_t lora_sync_word;

    // Forwarding mode: true = MQTT, false = HTTP
    bool use_mqtt;

    // MQTT
    char mqtt_broker[65];
    int  mqtt_port;
    char mqtt_topic[65];
    char mqtt_user[33];
    char mqtt_pass[65];

    // HTTP
    char http_url[129];

    // Gateway identity
    char gateway_id[33];
};

extern GatewayConfig gConfig;

bool configInit();
bool configLoad(GatewayConfig &cfg);
bool configSave(const GatewayConfig &cfg);
void configDefaults(GatewayConfig &cfg);
GatewayConfig& configGet();

#endif
