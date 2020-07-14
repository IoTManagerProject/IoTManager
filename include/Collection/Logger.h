#pragma once

#include <Arduino.h>

#include "Objects/LoggerTask.h"

typedef std::function<bool(LoggerTask*)> LoggerTaskHandler;

namespace Logger {
void init();
void add(const char* name, unsigned long period, size_t count);
void update();
void clear();
const String asJson();
void forEach(LoggerTaskHandler func);
};  // namespace Logger
