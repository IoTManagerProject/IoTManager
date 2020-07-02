#pragma once

#include <Arduino.h>

namespace Events {
// событие: имя + Set + номер
void fire(String name, String param);

void fire(String name);

}  // namespace Events