#pragma once

#include "SoftwareSerial.h"

extern void uartInit();
extern void uartHandle();
extern void parse(String& incStr);