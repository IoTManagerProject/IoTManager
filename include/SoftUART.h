#pragma once

#include "SoftwareSerial.h"

extern SoftwareSerial* myUART;

extern void uartInit();
extern void uartHandle();
extern void parse(String& incStr);