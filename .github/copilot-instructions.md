# Copilot Instructions for ESP32 LoRa-to-Internet Gateway

## Project Snapshot
- Target: TTGO LoRa32 v2.1 (ESP32 + SX1276 + SSD1306), Arduino framework, PlatformIO env `ttgo-lora32-v21`.
- Purpose: single-channel LoRa packet gateway that forwards to MQTT or HTTP, configured via captive-portal web UI.
- Main entrypoint is `src/main.cpp`; most behavior is cooperative in `loop()` (no RTOS task architecture).

## Build / Flash / Monitor
- Build: `platformio run`
- Upload: `platformio run --target upload`
- Serial monitor: `platformio device monitor -b 115200`
- Filesystem is LittleFS (`board_build.filesystem = littlefs`), config persists in `/config.json`.

## Runtime Architecture (read before edits)
- Config lifecycle: `configInit()` mounts LittleFS, loads `gConfig`, or writes defaults (`src/config.cpp`).
- Boot order in `setup()` matters: config -> display -> battery -> LoRa -> WiFi -> WiFi scan -> web server -> forwarder.
- `src/web_server.cpp` must not perform heavy reconfiguration directly; it sets volatile flags.
- `src/main.cpp` owns reconfiguration by consuming flags (`flagWifiChanged`, `flagLoraChanged`, `flagForwardChanged`, etc.).
- Data flow: `loraPoll()` -> update counters/status -> `forwarderSend()` when WiFi connected -> display refresh.

## Project-Specific Coding Patterns
- Use global `gConfig` as the source of truth for runtime config (`src/config.h`).
- After changing any config field from HTTP handlers, call `configSave(gConfig)` and set the matching flag.
- Keep networking responsive: avoid long blocking work in web handlers and `loop()` hot paths.
- Keep timed behavior via `millis()`-based intervals (examples in `src/main.cpp`: battery/display/MQTT loop cadence).
- Preserve fixed-size `char[]` config fields and `strlcpy` usage; do not replace with heap-heavy String storage in config structs.

## Integration Points
- LoRa radio: Sandeep Mistry LoRa library (`src/lora_radio.cpp`) with pins from `include/pins.h`.
- Web UI: ESPAsyncWebServer + AsyncTCP; HTML is assembled server-side from `src/web_server.cpp` + `src/web_pages.h`.
- Forwarding: MQTT via PubSubClient, HTTP via HTTPClient (`src/forwarder.cpp`), payload JSON built with ArduinoJson.
- Display: SSD1306Wire dashboard updates every second (`src/display.cpp`).
- Battery safety: GPIO35 ADC mapping to LiPo %, LoRa disabled below 5% (`src/battery.cpp`, enforced in `src/main.cpp`).

## Practical Guardrails for Changes
- If adding new settings: update `GatewayConfig`, defaults, load/save JSON keys, web form fields, and runtime apply path.
- If changing LoRa/forwarding behavior: ensure OLED counters/status and serial logs still reflect packet Rx/Fwd path.
- If touching WiFi/AP logic: preserve captive portal DNS behavior and 60s reconnect cadence in STA mode.
- Keep edits minimal and local to module boundaries (`config`, `wifi_manager`, `web_server`, `forwarder`, `lora_radio`, `display`, `battery`).

## Current Testing Reality
- There is no meaningful automated test suite in `test/`; validate by build + flash + serial logs + web portal smoke test.
