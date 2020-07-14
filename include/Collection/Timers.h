#include <Arduino.h>

#include "Objects/Timer.h"

namespace Timers {
Timer* add(const String& name, unsigned long time);
void update();
void erase(const String& name);
}  // namespace Timers