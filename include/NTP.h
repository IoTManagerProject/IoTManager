#pragma once
#include "Arduino.h"
#include "Global.h"

static const uint8_t days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

extern void breakEpochToTime(unsigned long epoch, Time_t& tm);
extern void ntpInit();
extern time_t getSystemTime();
extern void synchTime();
extern bool onDayChange();
extern const String getTimeLocal_hhmm();
extern const String getTimeLocal_hhmmss();
extern const String getDateTimeDotFormated();
extern const String getTodayDateDotFormated();
extern unsigned long strDateToUnix(String date);
extern const String getDateTimeDotFormatedFromUnix(unsigned long unixTime);
extern const String getTimeDotFormatedFromUnix(unsigned long unixTime);
extern unsigned long gmtTimeToLocal(unsigned long gmtTimestamp);
extern const String getDateDotFormatedFromUnix(unsigned long unixTime);
