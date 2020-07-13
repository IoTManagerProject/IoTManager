#pragma once

#include <Arduino.h>

#include "Logs/LoggerTask.h"

typedef std::function<bool(LoggerTask*)> LoggerTaskHandler;

namespace Logger {
void add(const char* name, unsigned long period, size_t count);
void init();
void loop();
void clear();
const String asJson();
void forEach(LoggerTaskHandler func);
};  // namespace Logger
