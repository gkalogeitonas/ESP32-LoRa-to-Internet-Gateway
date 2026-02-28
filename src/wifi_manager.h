#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include "config.h"

void   wifiInit(const GatewayConfig &cfg);
void   wifiLoop();
bool   wifiIsConnected();
bool   wifiIsAPMode();
String wifiGetIP();
void   wifiStartScan();
bool   wifiScanComplete();
String wifiScanResultsJson();

#endif
