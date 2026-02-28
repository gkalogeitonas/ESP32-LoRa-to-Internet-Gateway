#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include "config.h"
#include <vector>

struct ScannedNetwork {
    String ssid;
    int    rssi;
    bool   encrypted;
};

void   wifiInit(const GatewayConfig &cfg);
void   wifiLoop();
bool   wifiIsConnected();
bool   wifiIsAPMode();
String wifiGetIP();
void   wifiScanSync();
const std::vector<ScannedNetwork>& wifiGetNetworks();

#endif
