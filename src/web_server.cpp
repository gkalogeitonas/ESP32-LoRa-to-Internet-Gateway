#include "web_server.h"
#include "web_pages.h"
#include "config.h"
#include "wifi_manager.h"
#include "battery.h"
#include <ESPAsyncWebServer.h>

volatile bool flagWifiChanged = false;
volatile bool flagLoraChanged = false;
volatile bool flagForwardChanged = false;
volatile bool flagRestartRequested = false;
volatile bool flagRescanRequested = false;

extern unsigned long totalPacketsForwarded;
extern int currentBatteryPercent;

static AsyncWebServer server(80);

// Helper: add "selected" attribute if values match
static void addSelected(String &html, long current, long value) {
    if (current == value) html += " selected";
}

// Build the full page HTML with all values pre-populated server-side
static String buildPage(const char* msg = nullptr) {
    String html;
    html.reserve(5400);

    char syncWordBuf[8];
    snprintf(syncWordBuf, sizeof(syncWordBuf), "0x%02X", gConfig.lora_sync_word);

    html += PAGE_HEAD;

    // Flash message from a save action
    if (msg) {
        html += "<div class=\"msg\">";
        html += msg;
        html += "</div>";
    }

    // --- Status bar ---
    html += "<div class=\"card\"><div class=\"status-bar\">";
    html += "<div class=\"status-item\">WiFi: <span>";
    html += wifiIsConnected() ? "Connected" : (wifiIsAPMode() ? "AP Mode" : "Disconnected");
    html += "</span></div>";
    html += "<div class=\"status-item\">IP: <span>";
    html += wifiGetIP();
    html += "</span></div>";
    html += "<div class=\"status-item\">Batt: <span>";
    html += String(currentBatteryPercent);
    html += "%</span></div>";
    html += "<div class=\"status-item\">Pkts: <span>";
    html += String(totalPacketsForwarded);
    html += "</span></div>";
    html += "</div></div>";

    // --- WiFi form ---
    html += "<div class=\"card\"><h2>WiFi Settings</h2>";
    html += "<form method=\"POST\" action=\"/save/wifi\">";
    html += "<label>WiFi Network</label>";
    html += "<select name=\"ssid\">";
    html += "<option value=\"\">-- Select a network --</option>";

    const auto &nets = wifiGetNetworks();
    for (const auto &n : nets) {
        html += "<option value=\"";
        html += n.ssid;
        html += "\"";
        if (n.ssid == String(gConfig.wifi_ssid)) html += " selected";
        html += ">";
        html += n.ssid;
        html += " (";
        html += String(n.rssi);
        html += " dBm";
        if (n.encrypted) html += " *)";
        else html += ")";
        html += "</option>";
    }

    html += "</select>";
    html += "<label>Or enter manually</label>";
    html += "<input type=\"text\" name=\"ssid_manual\" placeholder=\"SSID\">";
    html += "<label>Password</label>";
    html += "<input type=\"password\" name=\"pass\">";
    html += "<input type=\"submit\" value=\"Save WiFi\">";
    html += "</form>";
    html += "<form method=\"POST\" action=\"/rescan\" style=\"margin-top:8px\">";
    html += "<input type=\"submit\" value=\"Rescan Networks\" style=\"background:#0f3460;color:#e0e0e0\">";
    html += "</form></div>";

    // --- LoRa form ---
    long freq = gConfig.lora_frequency;

    // Channel lookup
    static const long channels[] = {868100000,868300000,868500000,867100000,867300000,867500000,867700000,867900000};
    static const char* chNames[] = {"Ch0: 868.1 MHz","Ch1: 868.3 MHz","Ch2: 868.5 MHz","Ch3: 867.1 MHz","Ch4: 867.3 MHz","Ch5: 867.5 MHz","Ch6: 867.7 MHz","Ch7: 867.9 MHz"};
    bool isCustom = true;
    for (int i = 0; i < 8; i++) {
        if (freq == channels[i]) { isCustom = false; break; }
    }

    html += "<div class=\"card\"><h2>LoRa Settings</h2>";
    html += "<form method=\"POST\" action=\"/save/lora\">";

    html += "<div class=\"row\"><div>";
    html += "<label>Channel (EU868)</label>";
    html += "<select name=\"channel\" id=\"lora-channel\" onchange=\"uf()\">";
    for (int i = 0; i < 8; i++) {
        html += "<option value=\"";
        html += String(channels[i]);
        html += "\"";
        if (freq == channels[i]) html += " selected";
        html += ">";
        html += chNames[i];
        html += "</option>";
    }
    html += "<option value=\"custom\"";
    if (isCustom) html += " selected";
    html += ">Custom...</option>";
    html += "</select></div>";

    html += "<div><label>Spreading Factor</label><select name=\"sf\">";
    for (int sf = 7; sf <= 12; sf++) {
        html += "<option value=\"";
        html += String(sf);
        html += "\"";
        if (gConfig.lora_sf == sf) html += " selected";
        html += ">SF";
        html += String(sf);
        html += "</option>";
    }
    html += "</select></div></div>";

    html += "<div id=\"custom-freq-section\" style=\"display:";
    html += isCustom ? "block" : "none";
    html += "\"><label>Custom Frequency (Hz)</label>";
    html += "<input type=\"number\" name=\"frequency\" id=\"lora-freq\" value=\"";
    html += String(freq);
    html += "\"></div>";

    html += "<div class=\"row\"><div><label>Bandwidth</label><select name=\"bandwidth\">";
    long bws[] = {125000, 250000, 500000};
    const char* bwNames[] = {"125 kHz", "250 kHz", "500 kHz"};
    for (int i = 0; i < 3; i++) {
        html += "<option value=\"";
        html += String(bws[i]);
        html += "\"";
        addSelected(html, gConfig.lora_bandwidth, bws[i]);
        html += ">";
        html += bwNames[i];
        html += "</option>";
    }
    html += "</select></div>";

    html += "<div><label>Coding Rate</label><select name=\"coding_rate\">";
    for (int cr = 5; cr <= 8; cr++) {
        html += "<option value=\"";
        html += String(cr);
        html += "\"";
        if (gConfig.lora_coding_rate == cr) html += " selected";
        html += ">4/";
        html += String(cr);
        html += "</option>";
    }
    html += "</select></div></div>";

    html += "<label>Sync Word (hex 0x12 or decimal 18)</label>";
    html += "<input type=\"text\" name=\"sync_word\" value=\"";
    html += syncWordBuf;
    html += "\">";

    html += "<input type=\"submit\" value=\"Save LoRa\">";
    html += "</form></div>";

    // --- Forwarding form ---
    html += "<div class=\"card\"><h2>Data Forwarding</h2>";
    html += "<form method=\"POST\" action=\"/save/forward\">";

    html += "<div class=\"radio-group\">";
    html += "<label><input type=\"radio\" name=\"use_mqtt\" value=\"1\" onchange=\"tf()\"";
    if (gConfig.use_mqtt) html += " checked";
    html += "> MQTT</label>";
    html += "<label><input type=\"radio\" name=\"use_mqtt\" value=\"0\" onchange=\"tf()\"";
    if (!gConfig.use_mqtt) html += " checked";
    html += "> HTTP</label>";
    html += "</div>";

    html += "<div id=\"mqtt-section\" style=\"display:";
    html += gConfig.use_mqtt ? "block" : "none";
    html += "\">";
    html += "<div class=\"row\"><div><label>Broker</label>";
    html += "<input type=\"text\" name=\"mqtt_broker\" value=\"";
    html += gConfig.mqtt_broker;
    html += "\"></div>";
    html += "<div><label>Port</label>";
    html += "<input type=\"number\" name=\"mqtt_port\" value=\"";
    html += String(gConfig.mqtt_port);
    html += "\"></div></div>";
    html += "<label>Topic</label><input type=\"text\" name=\"mqtt_topic\" value=\"";
    html += gConfig.mqtt_topic;
    html += "\">";
    html += "<div class=\"row\"><div><label>Username</label>";
    html += "<input type=\"text\" name=\"mqtt_user\" value=\"";
    html += gConfig.mqtt_user;
    html += "\"></div><div><label>Password</label>";
    html += "<input type=\"password\" name=\"mqtt_pass\" value=\"";
    html += gConfig.mqtt_pass;
    html += "\"></div></div>";
    html += "</div>";

    html += "<div id=\"http-section\" style=\"display:";
    html += gConfig.use_mqtt ? "none" : "block";
    html += "\">";
    html += "<label>Endpoint URL (POST)</label>";
    html += "<input type=\"text\" name=\"http_url\" value=\"";
    html += gConfig.http_url;
    html += "\">";
    html += "</div>";

    html += "<label>Gateway ID</label><input type=\"text\" name=\"gateway_id\" value=\"";
    html += gConfig.gateway_id;
    html += "\">";
    html += "<input type=\"submit\" value=\"Save Forwarding\">";
    html += "</form></div>";

    // --- Restart ---
    html += "<div class=\"card\">";
    html += "<form method=\"POST\" action=\"/restart\">";
    html += "<input type=\"submit\" value=\"Restart Device\" class=\"danger\">";
    html += "</form></div>";

    html += PAGE_FOOT;

    return html;
}

// Saved-and-redirect message pages
static const char SAVED_REDIRECT[] PROGMEM =
    "<html><head><meta http-equiv=\"refresh\" content=\"2;url=/\"></head>"
    "<body style=\"background:#1a1a2e;color:#4caf50;font-family:sans-serif;text-align:center;padding-top:40vh\">"
    "<h2>%s</h2><p>Redirecting...</p></body></html>";

static String savedPage(const char* message) {
    char buf[300];
    snprintf(buf, sizeof(buf), SAVED_REDIRECT, message);
    return String(buf);
}

// --- Route handlers ---

static void handleRoot(AsyncWebServerRequest *request) {
    request->send(200, "text/html", buildPage());
}

static void handleSaveWifi(AsyncWebServerRequest *request) {
    // Prefer manual SSID, fall back to dropdown
    String ssid;
    if (request->hasParam("ssid_manual", true) &&
        request->getParam("ssid_manual", true)->value().length() > 0) {
        ssid = request->getParam("ssid_manual", true)->value();
    } else if (request->hasParam("ssid", true)) {
        ssid = request->getParam("ssid", true)->value();
    }

    if (ssid.length() == 0) {
        request->send(200, "text/html", buildPage("Error: No SSID provided"));
        return;
    }

    strlcpy(gConfig.wifi_ssid, ssid.c_str(), sizeof(gConfig.wifi_ssid));
    if (request->hasParam("pass", true)) {
        strlcpy(gConfig.wifi_pass, request->getParam("pass", true)->value().c_str(), sizeof(gConfig.wifi_pass));
    }
    configSave(gConfig);
    flagWifiChanged = true;

    request->send(200, "text/html", savedPage("WiFi saved. Reconnecting..."));
}

static void handleSaveLora(AsyncWebServerRequest *request) {
    // Channel or custom frequency
    if (request->hasParam("channel", true)) {
        String ch = request->getParam("channel", true)->value();
        if (ch != "custom") {
            gConfig.lora_frequency = ch.toInt();
        } else if (request->hasParam("frequency", true)) {
            gConfig.lora_frequency = request->getParam("frequency", true)->value().toInt();
        }
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
    if (request->hasParam("sync_word", true)) {
        String syncWord = request->getParam("sync_word", true)->value();
        char *endPtr = nullptr;
        long parsed = strtol(syncWord.c_str(), &endPtr, 0);
        if (endPtr != syncWord.c_str() && parsed >= 0 && parsed <= 255) {
            gConfig.lora_sync_word = (uint8_t)parsed;
        }
    }
    configSave(gConfig);
    flagLoraChanged = true;

    request->send(200, "text/html", savedPage("LoRa settings saved."));
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

    request->send(200, "text/html", savedPage("Forwarding config saved."));
}

static void handleRescan(AsyncWebServerRequest *request) {
    flagRescanRequested = true;
    request->send(200, "text/html", savedPage("Rescanning networks..."));
}

static void handleRestart(AsyncWebServerRequest *request) {
    request->send(200, "text/html", savedPage("Restarting device..."));
    flagRestartRequested = true;
}

// Captive portal detection endpoints
static void handleGenerate204(AsyncWebServerRequest *request) {
    request->send(204);
}

void webServerInit() {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/save/wifi", HTTP_POST, handleSaveWifi);
    server.on("/save/lora", HTTP_POST, handleSaveLora);
    server.on("/save/forward", HTTP_POST, handleSaveForward);
    server.on("/rescan", HTTP_POST, handleRescan);
    server.on("/restart", HTTP_POST, handleRestart);

    // Captive portal detection (Android, iOS, Windows, macOS)
    server.on("/generate_204", HTTP_GET, handleGenerate204);
    server.on("/hotspot-detect.html", HTTP_GET, handleRoot);
    server.on("/ncsi.txt", HTTP_GET, handleRoot);
    server.on("/connecttest.txt", HTTP_GET, handleRoot);
    server.on("/connectivity-check.html", HTTP_GET, handleRoot);

    server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(204);
    });

    // Fallback: redirect to root
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->redirect("/");
    });

    server.begin();
    Serial.println("[WebServer] Started on port 80");
}
