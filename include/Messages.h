#pragma once

#include <Arduino.h>

#include "Base/StringQueue.h"

namespace Messages {
void announce();

void loop();

void parseReceived(StringQueue* queue);

}