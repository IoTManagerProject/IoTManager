#pragma once
#include "Global.h"

#ifdef ESP8266
//эта библиотека встроена в ядро
#include "ESPAsyncUDP.h"
#else
#include "AsyncUDP.h"
#endif
extern AsyncUDP asyncUdp;

extern const String getThisDevice();
extern void addThisDeviceToList();
extern void asyncUdpInit();
extern void udpPacketParse(String& data);
extern void jsonMergeArrays(String& existJson, String& incJson);