#include "web_server.h"
#include "web_pages.h"
#include "config.h"
#include "wifi_manager.h"
#include "battery.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

volatile bool flagWifiChanged = false;
volatile bool flagLoraChanged = false;
volatile bool flagForwardChanged = false;
volatile bool flagRestartRequested = false;

// Externals from main.cpp for status reporting
extern unsigned long totalPacketsForwarded;
extern int currentBatteryPercent;

static AsyncWebServer server(80);

static void handleRoot(AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", PAGE_HTML);
}

static void handleStatus(AsyncWebServerRequest *request) {
    JsonDocument doc;

    doc["wifi"] = wifiIsConnected() ? "Connected" : (wifiIsAPMode() ? "AP Mode" : "Disconnected");
    doc["ip"]   = wifiGetIP();
    doc["battery"] = currentBatteryPercent;
    doc["packets"] = totalPacketsForwarded;

    // Include current config for form population
    JsonObject cfg = doc["config"].to<JsonObject>();
    cfg["wifi_ssid"]       = gConfig.wifi_ssid;
    cfg["lora_frequency"]  = gConfig.lora_frequency;
    cfg["lora_sf"]         = gConfig.lora_sf;
    cfg["lora_bandwidth"]  = gConfig.lora_bandwidth;
    cfg["lora_coding_rate"] = gConfig.lora_coding_rate;
    cfg["use_mqtt"]        = gConfig.use_mqtt;
    cfg["mqtt_broker"]     = gConfig.mqtt_broker;
    cfg["mqtt_port"]       = gConfig.mqtt_port;
    cfg["mqtt_topic"]      = gConfig.mqtt_topic;
    cfg["mqtt_user"]       = gConfig.mqtt_user;
    cfg["http_url"]        = gConfig.http_url;
    cfg["gateway_id"]      = gConfig.gateway_id;

    String output;
    serializeJson(doc, output);
    request->send(200, "application/json", output);
}

static void handleScan(AsyncWebServerRequest *request) {
    if (!wifiScanComplete()) {
        wifiStartScan();
        request->send(200, "application/json", "[]");
        return;
    }
    request->send(200, "application/json", wifiScanResultsJson());
}

static void handleSaveWifi(AsyncWebServerRequest *request) {
    if (!request->hasParam("ssid", true)) {
        request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing SSID\"}");
        return;
    }

    strlcpy(gConfig.wifi_ssid, request->getParam("ssid", true)->value().c_str(), sizeof(gConfig.wifi_ssid));
    strlcpy(gConfig.wifi_pass, request->getParam("pass", true)->value().c_str(), sizeof(gConfig.wifi_pass));
    configSave(gConfig);

    flagWifiChanged = true;
    request->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"WiFi saved. Reconnecting...\"}");
}

static void handleSaveLora(AsyncWebServerRequest *request) {
    if (request->hasParam("frequency", true)) {
        gConfig.lora_frequency = request->getParam("frequency", true)->value().toInt();
    }
    if (request->hasParam("sf", true)) {
        gConfig.lora_sf = request->getParam("sf", true)->value().toInt();
    }
    if (request->hasParam("bandwidth", true)) {
        gConfig.lora_bandwidth = request->getParam("bandwidth", true)->value().toInt();
    }
    if (request->hasParam("coding_rate", true)) {
        gConfig.lora_coding_rate = request->getParam("coding_rate", true)->value().toInt();
    }
    configSave(gConfig);

    flagLoraChanged = true;
    request->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"LoRa settings saved.\"}");
}

static void handleSaveForward(AsyncWebServerRequest *request) {
    if (request->hasParam("use_mqtt", true)) {
        gConfig.use_mqtt = request->getParam("use_mqtt", true)->value() == "1";
    }
    if (request->hasParam("mqtt_broker", true)) {
        strlcpy(gConfig.mqtt_broker, request->getParam("mqtt_broker", true)->value().c_str(), sizeof(gConfig.mqtt_broker));
    }
    if (request->hasParam("mqtt_port", true)) {
        gConfig.mqtt_port = request->getParam("mqtt_port", true)->value().toInt();
    }
    if (request->hasParam("mqtt_topic", true)) {
        strlcpy(gConfig.mqtt_topic, request->getParam("mqtt_topic", true)->value().c_str(), sizeof(gConfig.mqtt_topic));
    }
    if (request->hasParam("mqtt_user", true)) {
        strlcpy(gConfig.mqtt_user, request->getParam("mqtt_user", true)->value().c_str(), sizeof(gConfig.mqtt_user));
    }
    if (request->hasParam("mqtt_pass", true)) {
        strlcpy(gConfig.mqtt_pass, request->getParam("mqtt_pass", true)->value().c_str(), sizeof(gConfig.mqtt_pass));
    }
    if (request->hasParam("http_url", true)) {
        strlcpy(gConfig.http_url, request->getParam("http_url", true)->value().c_str(), sizeof(gConfig.http_url));
    }
    if (request->hasParam("gateway_id", true)) {
        strlcpy(gConfig.gateway_id, request->getParam("gateway_id", true)->value().c_str(), sizeof(gConfig.gateway_id));
    }
    configSave(gConfig);

    flagForwardChanged = true;
    request->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Forwarding config saved.\"}");
}

static void handleRestart(AsyncWebServerRequest *request) {
    request->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Restarting...\"}");
    flagRestartRequested = true;
}

void webServerInit() {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/status", HTTP_GET, handleStatus);
    server.on("/scan", HTTP_GET, handleScan);
    server.on("/save/wifi", HTTP_POST, handleSaveWifi);
    server.on("/save/lora", HTTP_POST, handleSaveLora);
    server.on("/save/forward", HTTP_POST, handleSaveForward);
    server.on("/restart", HTTP_POST, handleRestart);

    // Captive portal: redirect all unknown requests to root
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->redirect("/");
    });

    server.begin();
    Serial.println("[WebServer] Started on port 80");
}
