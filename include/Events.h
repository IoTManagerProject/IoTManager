#pragma once

#include <Arduino.h>

#include "Base/StringQueue.h"

namespace Events {
StringQueue* get();

void fire(String name, String param);

void fire(const String& name);

}  // namespace Events