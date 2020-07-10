#pragma once

#include <Arduino.h>

#include "Base/Named.h"
#include "Base/StringQueue.h"

namespace Events {
StringQueue* get();

void fire(String name, String param);
void fire(Named* obj);
void fire(const String& name);

}  // namespace Events