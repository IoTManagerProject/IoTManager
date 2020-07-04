#pragma once

#include <Arduino.h>

#include "Base/StringQueue.h"

namespace Broadcast {
void send(const String header, const String data);
void init();
void loop();
StringQueue* received();
}  // namespace Broadcast
