#pragma once

#include <Arduino.h>

#include "Consts.h"
#include "Base/StringQueue.h"
#include "StreamString.h"

namespace Messages {
void post(const BroadcastMessage_t, const String&);

void outcome(String& str);
bool available();

void loop();
void income(const String str);
void parse(const String& buf);
}  // namespace Messages