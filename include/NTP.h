#pragma once
#include "Arduino.h"
#include "Global.h"

static const uint8_t days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

extern void breakEpochToTime(unsigned long epoch, Time_t& tm);
extern void ntpInit();
extern time_t getSystemTime();
extern void synchTime();
extern const String getTimeLocal_hhmm();
extern const String getTimeLocal_hhmmss();
extern const String getDateTimeDotFormated();
extern unsigned long strDateToUnix(String date);
