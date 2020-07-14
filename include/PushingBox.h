#pragma once

#include <Arduino.h>

namespace PushingBox {
void init();
void post(const String& title, const String& body);
}