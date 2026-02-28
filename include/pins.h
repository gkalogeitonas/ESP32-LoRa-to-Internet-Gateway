#ifndef PINS_H
#define PINS_H

// LoRa SPI pins (TTGO LoRa32 v2.1, proven in receiver.cpp)
#define LORA_SCK_PIN    5
#define LORA_MISO_PIN   19
#define LORA_MOSI_PIN   27
#define LORA_SS_PIN     18
#define LORA_RST_PIN    14
#define LORA_DIO0_PIN   26

// OLED I2C pins
#define OLED_SDA_PIN    21
#define OLED_SCL_PIN    22
#define OLED_ADDR       0x3c

// Battery ADC (ADC1_CH7, safe with WiFi active)
#define BATTERY_PIN     35

// Built-in LED
#define LED_PIN         25

#endif
