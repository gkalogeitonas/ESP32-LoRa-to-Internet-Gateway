#ifndef BATTERY_H
#define BATTERY_H

#include <Arduino.h>

void  batteryInit();
int   batteryReadPercent();
float batteryReadVoltage();
bool  batteryIsCritical();

#endif
