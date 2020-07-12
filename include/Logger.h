#pragma once

#include <Arduino.h>

#include <functional>

typedef std::function<void(const String name, unsigned long time, float value)> EntryHandler;

namespace Logger {
void add(String name, unsigned long period, size_t count);
void init();
void loop();
void clear();
void publishTasks(EntryHandler func);
const String asJson();
};  // namespace Logger
