#pragma once

#include <Arduino.h>

namespace Logger {
void add(String name, unsigned long period, size_t count);
void init();
void loop();
void clear();
void publish();
const String asJson();
};  // namespace Logger
