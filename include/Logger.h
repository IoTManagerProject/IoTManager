#pragma once

#include <Arduino.h>

#include "MqttWriter.h"

namespace Logger {
void add(const char* name, unsigned long period, size_t count);
void init();
void loop();
void clear();
void publish(Writer*);
const String asJson();
};  // namespace Logger
