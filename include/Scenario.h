#pragma once

#include <Arduino.h>

#include "Objects/EventQueue.h"

namespace Scenario {

void init();

void load();

void process(EventQueue* events);

void enable(size_t num, boolean enable);

bool isEnabled(size_t num);

}  // namespace Scenario