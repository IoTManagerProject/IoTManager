#pragma once

#ifdef ESP8266
#include <SoftwareSerial.h>
extern SoftwareSerial* myUART;
#else
#include <HardwareSerial.h>
extern Stream* myUART;
#endif