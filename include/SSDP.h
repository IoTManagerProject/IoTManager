#pragma once

#include <Arduino.h>

#ifdef SSDP_EN
extern void SsdpInit();
extern String xmlNode(String tags, String data);
extern String decToHex(uint32_t decValue, byte desiredStringLength);
#endif