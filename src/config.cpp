#include "config.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

GatewayConfig gConfig;

void configDefaults(GatewayConfig &cfg) {
    memset(&cfg, 0, sizeof(GatewayConfig));

    // WiFi: empty (triggers AP mode on first boot)
    cfg.wifi_ssid[0] = '\0';
    cfg.wifi_pass[0] = '\0';

    // LoRa defaults matching receiver.cpp
    cfg.lora_frequency  = 868000000;
    cfg.lora_sf         = 10;
    cfg.lora_bandwidth  = 125000;
    cfg.lora_coding_rate = 5;
    cfg.lora_sync_word  = 0x12;

    // Forwarding: MQTT by default
    cfg.use_mqtt = true;

    // MQTT defaults
    cfg.mqtt_broker[0] = '\0';
    cfg.mqtt_port      = 1883;
    strlcpy(cfg.mqtt_topic, "lora/gateway", sizeof(cfg.mqtt_topic));
    cfg.mqtt_user[0] = '\0';
    cfg.mqtt_pass[0] = '\0';

    // HTTP
    cfg.http_url[0] = '\0';

    // Gateway ID
    strlcpy(cfg.gateway_id, "lora-gw-001", sizeof(cfg.gateway_id));
}

bool configLoad(GatewayConfig &cfg) {
    File file = LittleFS.open("/config.json", "r");
    if (!file) {
        Serial.println("[Config] No config.json found");
        return false;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, file);
    file.close();

    if (err) {
        Serial.printf("[Config] Parse error: %s\n", err.c_str());
        return false;
    }

    strlcpy(cfg.wifi_ssid, doc["wifi_ssid"] | "", sizeof(cfg.wifi_ssid));
    strlcpy(cfg.wifi_pass, doc["wifi_pass"] | "", sizeof(cfg.wifi_pass));

    cfg.lora_frequency   = doc["lora_frequency"]  | 868000000L;
    cfg.lora_sf          = doc["lora_sf"]          | 10;
    cfg.lora_bandwidth   = doc["lora_bandwidth"]   | 125000L;
    cfg.lora_coding_rate = doc["lora_coding_rate"] | 5;
    cfg.lora_sync_word   = doc["lora_sync_word"]   | 0x12;

    cfg.use_mqtt = doc["use_mqtt"] | true;

    strlcpy(cfg.mqtt_broker, doc["mqtt_broker"] | "", sizeof(cfg.mqtt_broker));
    cfg.mqtt_port = doc["mqtt_port"] | 1883;
    strlcpy(cfg.mqtt_topic, doc["mqtt_topic"] | "lora/gateway", sizeof(cfg.mqtt_topic));
    strlcpy(cfg.mqtt_user, doc["mqtt_user"] | "", sizeof(cfg.mqtt_user));
    strlcpy(cfg.mqtt_pass, doc["mqtt_pass"] | "", sizeof(cfg.mqtt_pass));

    strlcpy(cfg.http_url, doc["http_url"] | "", sizeof(cfg.http_url));

    strlcpy(cfg.gateway_id, doc["gateway_id"] | "lora-gw-001", sizeof(cfg.gateway_id));

    Serial.println("[Config] Loaded config.json");
    return true;
}

bool configSave(const GatewayConfig &cfg) {
    JsonDocument doc;

    doc["wifi_ssid"]       = cfg.wifi_ssid;
    doc["wifi_pass"]       = cfg.wifi_pass;

    doc["lora_frequency"]  = cfg.lora_frequency;
    doc["lora_sf"]         = cfg.lora_sf;
    doc["lora_bandwidth"]  = cfg.lora_bandwidth;
    doc["lora_coding_rate"] = cfg.lora_coding_rate;
    doc["lora_sync_word"]  = cfg.lora_sync_word;

    doc["use_mqtt"]        = cfg.use_mqtt;

    doc["mqtt_broker"]     = cfg.mqtt_broker;
    doc["mqtt_port"]       = cfg.mqtt_port;
    doc["mqtt_topic"]      = cfg.mqtt_topic;
    doc["mqtt_user"]       = cfg.mqtt_user;
    doc["mqtt_pass"]       = cfg.mqtt_pass;

    doc["http_url"]        = cfg.http_url;

    doc["gateway_id"]      = cfg.gateway_id;

    File file = LittleFS.open("/config.json", "w");
    if (!file) {
        Serial.println("[Config] Failed to open config.json for writing");
        return false;
    }

    serializeJson(doc, file);
    file.close();

    Serial.println("[Config] Saved config.json");
    return true;
}

bool configInit() {
    if (!LittleFS.begin(true)) {
        Serial.println("[Config] LittleFS mount failed");
        return false;
    }
    Serial.println("[Config] LittleFS mounted");

    if (!configLoad(gConfig)) {
        Serial.println("[Config] Loading defaults");
        configDefaults(gConfig);
        configSave(gConfig);
    }

    return true;
}

GatewayConfig& configGet() {
    return gConfig;
}
