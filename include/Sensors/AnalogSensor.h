#pragma once

#include <Arduino.h>

namespace AnalogSensor {
void add(const String& name, const String& pin, const String& min_value, const String& max_value, const String& min_deg, const String& max_deg);
void loop();
}  // namespace AnalogSensor