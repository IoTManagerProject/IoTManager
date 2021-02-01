#include "Consts.h"
#ifdef MYSENSORS
#pragma once
#include "Global.h"
extern void loopMySensorsExecute();
extern String sensorType(int index);
extern void test(char* inputString);
#endif