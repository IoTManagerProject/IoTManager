#pragma once
#include <Arduino.h>
#include "Global.h"

extern void initSt();
extern String updateDevicePsn(String lat, String lon, String accur);
extern String updateDeviceStatus();
extern String addNewDevice();

extern void decide();
extern void getPsn();
//extern void updateDeviceList();
//extern void saveId(String file, int id);
//extern int getId(String file);