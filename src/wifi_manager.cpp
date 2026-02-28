#include "wifi_manager.h"
#include <WiFi.h>
#include <DNSServer.h>
#include <ArduinoJson.h>

static bool _apMode = false;
static DNSServer _dnsServer;
static unsigned long _lastReconnectAttempt = 0;
static const unsigned long RECONNECT_INTERVAL = 60000;
static const unsigned long CONNECT_TIMEOUT = 15000;

static void startAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("LoRa-GW-Setup");
    delay(100);

    _dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
    _dnsServer.start(53, "*", WiFi.softAPIP());

    _apMode = true;
    Serial.printf("[WiFi] AP started: %s IP: %s\n",
                  "LoRa-GW-Setup", WiFi.softAPIP().toString().c_str());
}

static bool connectSTA(const GatewayConfig &cfg) {
    if (strlen(cfg.wifi_ssid) == 0) return false;

    Serial.printf("[WiFi] Connecting to %s...\n", cfg.wifi_ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(cfg.wifi_ssid, cfg.wifi_pass);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start) < CONNECT_TIMEOUT) {
        delay(250);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
        return true;
    }

    Serial.println("[WiFi] Connection failed");
    return false;
}

void wifiInit(const GatewayConfig &cfg) {
    WiFi.disconnect(true);
    delay(100);

    _apMode = false;

    if (!connectSTA(cfg)) {
        startAP();
    }
}

void wifiLoop() {
    if (_apMode) {
        _dnsServer.processNextRequest();
        return;
    }

    // STA mode: check connection and attempt reconnect
    if (WiFi.status() != WL_CONNECTED) {
        unsigned long now = millis();
        if (now - _lastReconnectAttempt >= RECONNECT_INTERVAL) {
            _lastReconnectAttempt = now;
            Serial.println("[WiFi] Attempting reconnect...");
            WiFi.reconnect();
        }
    }
}

bool wifiIsConnected() {
    return !_apMode && WiFi.status() == WL_CONNECTED;
}

bool wifiIsAPMode() {
    return _apMode;
}

String wifiGetIP() {
    if (_apMode) {
        return WiFi.softAPIP().toString();
    }
    return WiFi.localIP().toString();
}

void wifiStartScan() {
    WiFi.scanNetworks(true, false);
    Serial.println("[WiFi] Scan started");
}

bool wifiScanComplete() {
    return WiFi.scanComplete() >= 0;
}

String wifiScanResultsJson() {
    int n = WiFi.scanComplete();
    if (n < 0) return "[]";

    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();

    for (int i = 0; i < n; i++) {
        JsonObject net = arr.add<JsonObject>();
        net["ssid"] = WiFi.SSID(i);
        net["rssi"] = WiFi.RSSI(i);
        net["enc"]  = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
    }

    WiFi.scanDelete();

    String result;
    serializeJson(doc, result);
    return result;
}
