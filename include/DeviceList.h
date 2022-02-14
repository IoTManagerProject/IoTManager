#pragma once
#include "Global.h"
#ifdef ESP8266
#include "ESPAsyncUDP.h"
#else
#include "AsyncUDP.h"
#endif
extern AsyncUDP asyncUdp;

extern const String getThisDevice();
extern void addThisDeviceToList();
extern void asyncUdpInit();
extern String uint8tToString(uint8_t* data, size_t len);
extern bool udpPacketValidation(String& data);
extern void udpPacketParse(String& data);
extern void jsonMergeArrays(String& existJson, String& incJson);