#include "Consts.h"
#ifdef MYSENSORS
#pragma once
#include "Global.h"
extern void loopMySensorsExecute();
extern void sensorType(int index, int &num, String &widget, String &descr);
extern void test(char* inputString);
#endif