#pragma once
//#include "/lib/WifiLocation.h"
#include <Arduino.h>
#include "Global.h"

extern void initSt();
extern void updateDevice(String lat, String lon, String accur, String uptime, String firm);
extern void addNewDevice(String model);
extern void updateDeviceList(String model, String firmVer);
extern void createNewDevJson(String& json, String model);
//extern WifiLocation location();