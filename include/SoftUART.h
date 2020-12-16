#pragma once
#include "Consts.h"
#ifdef uartEnable
#include "SoftwareSerial.h"

extern SoftwareSerial* myUART;

extern void uartInit();
extern void uartHandle();
extern void parse(String& incStr);
#endif