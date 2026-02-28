#include "battery.h"
#include "pins.h"

// LiPo discharge curve: voltage (V) to percentage
static const float BATT_V[] = { 3.00, 3.50, 3.60, 3.70, 3.75, 3.80, 3.85, 3.90, 3.95, 4.00, 4.05, 4.10, 4.15, 4.20 };
static const int   BATT_P[] = {    0,    5,   10,   20,   30,   40,   50,   60,   70,   80,   85,   90,   95,  100 };
static const int   BATT_TABLE_SIZE = sizeof(BATT_V) / sizeof(BATT_V[0]);

void batteryInit() {
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
    pinMode(BATTERY_PIN, INPUT);
}

float batteryReadVoltage() {
    uint32_t sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += analogRead(BATTERY_PIN);
    }
    float adcAvg = sum / 16.0f;
    // TTGO LoRa32 v2.1 has 100K/100K voltage divider on GPIO35
    float voltage = (adcAvg / 4095.0f) * 3.3f * 2.0f;
    return voltage;
}

int batteryReadPercent() {
    float voltage = batteryReadVoltage();

    if (voltage <= BATT_V[0]) return BATT_P[0];
    if (voltage >= BATT_V[BATT_TABLE_SIZE - 1]) return BATT_P[BATT_TABLE_SIZE - 1];

    // Piecewise linear interpolation
    for (int i = 0; i < BATT_TABLE_SIZE - 1; i++) {
        if (voltage >= BATT_V[i] && voltage <= BATT_V[i + 1]) {
            float ratio = (voltage - BATT_V[i]) / (BATT_V[i + 1] - BATT_V[i]);
            return BATT_P[i] + (int)(ratio * (BATT_P[i + 1] - BATT_P[i]));
        }
    }

    return 0;
}

bool batteryIsCritical() {
    return batteryReadPercent() < 5;
}
