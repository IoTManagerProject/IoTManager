#ifdef GATE_MODE
#pragma once

#include "Global.h"

extern void nodeTimesInit();
extern void publishTimes();
extern void publishNodeInfo(int minutes, String& key);
#endif