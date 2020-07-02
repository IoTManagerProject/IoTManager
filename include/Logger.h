#pragma once

#include "Global.h"
namespace Logger {
void add(String name, unsigned long period, size_t count);
void init();
void loop();
void clear();
void publish();
};  // namespace Logger
