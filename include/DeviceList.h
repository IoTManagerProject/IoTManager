#pragma once
#include "Global.h"
#include "ESPAsyncUDP.h"
extern AsyncUDP asyncUdp;

extern void addThisDeviceToList();
extern void asyncUdpInit();
extern String uint8tToString(uint8_t* data, size_t len);
extern bool udpPacketValidation(String& data);
extern void udpPacketParse(String& data);
extern void jsonMergeArrays(String& arr1, String& arr2);