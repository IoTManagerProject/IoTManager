#pragma once

#include <Arduino.h>

extern int timeToMin(String Time);
extern String GetDataDigital();
extern String GetDate();
extern String GetTimeWOsec();
extern String GetTime();
extern String GetTimeUnix();
extern void reconfigTime();
extern void saveConfig();
extern String GetTimeUnix();
extern void time_check();
