#pragma once
#include "Consts.h"
#ifdef uartEnable
#include "SoftwareSerial.h"

#ifdef ESP8266
#include <SoftwareSerial.h>
extern SoftwareSerial* myUART;
#else
#include <HardwareSerial.h>
extern HardwareSerial* myUART;
#endif


extern void uartInit();
extern void uartHandle();
extern void parse(String& incStr);
#endif