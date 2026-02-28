#include "forwarder.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

static WiFiClient _espClient;
static PubSubClient _mqttClient(_espClient);

static String buildPayload(const LoRaPacket &pkt, const GatewayConfig &cfg) {
    JsonDocument doc;

    doc["gateway_id"]       = cfg.gateway_id;
    doc["timestamp"]        = pkt.timestamp;
    doc["rssi"]             = pkt.rssi;
    doc["snr"]              = pkt.snr;
    doc["spreading_factor"] = pkt.sf;
    doc["frequency"]        = pkt.frequency;
    doc["packet_size"]      = pkt.size;

    // Raw payload as hex string
    String hexPayload;
    hexPayload.reserve(pkt.size * 2);
    for (int i = 0; i < pkt.size; i++) {
        char buf[3];
        snprintf(buf, sizeof(buf), "%02X", pkt.raw[i]);
        hexPayload += buf;
    }
    doc["raw_payload"] = hexPayload;

    String output;
    serializeJson(doc, output);
    return output;
}

static bool mqttSend(const String &payload, const GatewayConfig &cfg) {
    if (!_mqttClient.connected()) {
        const char* user = strlen(cfg.mqtt_user) > 0 ? cfg.mqtt_user : nullptr;
        const char* pass = strlen(cfg.mqtt_pass) > 0 ? cfg.mqtt_pass : nullptr;
        if (!_mqttClient.connect(cfg.gateway_id, user, pass)) {
            Serial.printf("[MQTT] Connect failed, rc=%d\n", _mqttClient.state());
            return false;
        }
        Serial.println("[MQTT] Connected");
    }

    bool ok = _mqttClient.publish(cfg.mqtt_topic, payload.c_str());
    if (ok) {
        Serial.println("[MQTT] Published");
    } else {
        Serial.println("[MQTT] Publish failed");
    }
    return ok;
}

static bool httpSend(const String &payload, const GatewayConfig &cfg) {
    if (strlen(cfg.http_url) == 0) {
        Serial.println("[HTTP] No URL configured");
        return false;
    }

    HTTPClient http;
    http.begin(cfg.http_url);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(5000);

    int code = http.POST(payload);
    http.end();

    bool ok = (code >= 200 && code < 300);
    Serial.printf("[HTTP] POST %s -> %d\n", cfg.http_url, code);
    return ok;
}

void forwarderInit(const GatewayConfig &cfg) {
    if (cfg.use_mqtt && strlen(cfg.mqtt_broker) > 0) {
        _mqttClient.setServer(cfg.mqtt_broker, cfg.mqtt_port);
        _mqttClient.setBufferSize(512);
        Serial.printf("[Forwarder] MQTT mode: %s:%d topic=%s\n",
                      cfg.mqtt_broker, cfg.mqtt_port, cfg.mqtt_topic);
    } else if (!cfg.use_mqtt) {
        Serial.printf("[Forwarder] HTTP mode: %s\n", cfg.http_url);
    } else {
        Serial.println("[Forwarder] No forwarding configured yet");
    }
}

void forwarderLoop() {
    if (gConfig.use_mqtt && _mqttClient.connected()) {
        _mqttClient.loop();
    }
}

bool forwarderSend(const LoRaPacket &pkt, const GatewayConfig &cfg) {
    String payload = buildPayload(pkt, cfg);
    Serial.printf("[Forwarder] Payload: %s\n", payload.c_str());

    if (cfg.use_mqtt) {
        return mqttSend(payload, cfg);
    } else {
        return httpSend(payload, cfg);
    }
}

void forwarderUpdateConfig(const GatewayConfig &cfg) {
    if (_mqttClient.connected()) {
        _mqttClient.disconnect();
    }
    forwarderInit(cfg);
}
