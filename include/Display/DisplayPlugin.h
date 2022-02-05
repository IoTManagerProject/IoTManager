#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#define DISPLAY_REFRESH_ms 1000
#define PAGE_CHANGE_ms 5000

#define D_LOG(fmt, ...) Serial.printf((PGM_P)PSTR(fmt), ##__VA_ARGS__)

namespace DisplayPlugin {
void show(const String& data, const String& meta);
}  // namespace DisplayPlugin