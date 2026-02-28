#ifndef FORWARDER_H
#define FORWARDER_H

#include <Arduino.h>
#include "config.h"
#include "lora_radio.h"

void forwarderInit(const GatewayConfig &cfg);
void forwarderLoop();
bool forwarderSend(const LoRaPacket &pkt, const GatewayConfig &cfg);
void forwarderUpdateConfig(const GatewayConfig &cfg);

#endif
