#pragma once
#include "WifiLocation.h"
#include <Arduino.h>
#include "Global.h"

extern void initSt();
extern void updateDevicePsn(String lat, String lon, String accur, String uptime, String firm);
extern void updateDeviceStatus(String uptime, String firm);
extern void getPsn();
extern void addNewDevice(String model);
extern void updateDeviceList(String model, String firmVer);
extern void createNewDevJson(String& json, String model);
//extern WifiLocation location();