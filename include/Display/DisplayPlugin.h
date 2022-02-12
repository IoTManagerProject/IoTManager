#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>


namespace DisplayPlugin {
void show(const String& data, const String& param);
}  // namespace DisplayPlugin