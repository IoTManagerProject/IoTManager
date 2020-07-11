#pragma once

#include <Arduino.h>

#include "Consts.h"
#include "Base/StringQueue.h"

namespace Messages {
void post(const BroadcastMessage_t, const String&);

void outcome(String& str);
bool available();

void loop();
void income(const String& str);
void parse(String buf);
}  // namespace Messages