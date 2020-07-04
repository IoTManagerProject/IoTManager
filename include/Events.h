#pragma once

#include <Arduino.h>

#include "Base/StringQueue.h"

namespace Events {
StringQueue* get();

// событие: имя + Set + номер
void fire(String name, String param);

void fire(String name);

}  // namespace Events