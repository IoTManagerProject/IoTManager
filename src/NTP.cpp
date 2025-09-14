#include "NTP.h"

#if defined(LIBRETINY)
#include "lwip/apps/sntp.h"
#endif

#include "Global.h"
#include "utils/SerialPrint.h"

void ntpInit() {
#if  defined(LIBRETINY)
    if (sntp_enabled()) {
        sntp_stop();
    }
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, jsonReadStr(settingsFlashJson, F("ntp")).c_str());
    sntp_setservername(1, "pool.ntp.org");
    sntp_setservername(2, "ru.pool.ntp.org");
    sntp_init();
#endif
    synchTime();

    ts.add(
        TIME, 1000, [&](void*) {
            unixTime = getSystemTime();
            //SerialPrint("I", F("NTP"), "TIME " + String(unixTime));
            if (unixTime < MIN_DATETIME) {
                isTimeSynch = false;
                // SerialPrint("E", "NTP", "Time not synched");
                jsonWriteInt(errorsHeapJson, F("tme1"), 1);
                synchTime();

                // проверяем присутствие RTC с батарейкой и получаем время при наличии
#ifdef mod_RtcDriver
                if (rtcItem) {
                    unixTime = rtcItem->getRtcUnixTime();
                } 
                else
#endif
                    return; // ToDo разобраться: если нет железных часов дальше ничего не делать ???
            }

            unixTimeShort = unixTime - START_DATETIME;
            jsonWriteInt(errorsHeapJson, F("tme1"), 0);
            breakEpochToTime(unixTime + jsonReadInt(settingsFlashJson, F("timezone")) * 60 * 60, _time_local);
            breakEpochToTime(unixTime, _time_utc);
            isTimeSynch = true;
            String timenow = getTimeLocal_hhmm();
            static String prevTime;
            if (prevTime != timenow) {
                prevTime = timenow;
                String dateAndTime = getDateTimeDotFormated();
                dateAndTime = deleteToMarkerLast(dateAndTime, ":");
                jsonWriteStr_(errorsHeapJson, F("timenow"), dateAndTime);
                SerialPrint("I", F("NTP"), "✔ " + dateAndTime);
                onDayChange();
            }
            _time_isTrust = true;  // доверяем значению времени
        },
        nullptr, true);

    SerialPrint("I", F("NTP"), F("Handle time init"));
}

void synchTime() {
#if defined LIBRETINY    
  // force resync
  if (sntp_enabled()) {
    sntp_stop();
    }
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, jsonReadStr(settingsFlashJson, F("ntp")).c_str());
    sntp_setservername(1, "pool.ntp.org");
    sntp_setservername(2, "ru.pool.ntp.org");
    sntp_init();

#else
    configTime(0, 0, "pool.ntp.org", "ru.pool.ntp.org", jsonReadStr(settingsFlashJson, F("ntp")).c_str());
#endif
}

//событие смены даты
bool onDayChange() {
    bool changed = false;
    String currentDate = getTodayDateDotFormated();
    if (!firstTimeInit) {
        if (prevDate != currentDate) {
            changed = true;
            SerialPrint("i", F("NTP"), F("Change day core event"));
            //установим новую дату во всех графиках системы
            for (std::list<IoTItem*>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
                (*it)->setTodayDate();
            }
        }
    }
    firstTimeInit = false;
    prevDate = currentDate;
    return changed;
}

unsigned long gmtTimeToLocal(unsigned long gmtTimestamp) {
    return gmtTimestamp + (jsonReadInt(settingsFlashJson, F("timezone")) * 60 * 60);
}

time_t getSystemTime() {
    timeval tv{0, 0};
    timezone tz = timezone{0, 0};
    time_t epoch = 0;
    if (gettimeofday(&tv, &tz) != -1) {
        epoch = tv.tv_sec;
    }
    return epoch;
}

void breakEpochToTime(unsigned long epoch, Time_t& tm) {
    // break the given time_input into time components
    // this is a more compact version of the C library localtime function

    unsigned long time = epoch;
    tm.second = time % 60;
    time /= 60;  // now it is minutes
    tm.minute = time % 60;
    time /= 60;  // now it is hours
    tm.hour = time % 24;
    time /= 24;  // now it is days
    tm.days = time;
    tm.day_of_week = ((time + 4) % 7) + 1;  // Sunday is day 1

    uint8_t year = 0;
    unsigned long days = 0;

    while ((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
        year++;
    }
    tm.year = year - 30;

    days -= LEAP_YEAR(year) ? 366 : 365;
    time -= days;  // now it is days in this year, starting at 0
    tm.day_of_year = time;

    uint8_t month;
    uint8_t month_length;
    for (month = 0; month < 12; month++) {
        if (1 == month) {  // february
            if (LEAP_YEAR(year)) {
                month_length = 29;
            } else {
                month_length = 28;
            }
        } else {
            month_length = days_in_month[month];
        }

        if (time >= month_length) {
            time -= month_length;
        } else {
            break;
        }
    }
    tm.month = month + 1;        // jan is month 1
    tm.day_of_month = time + 1;  // day of month
    tm.valid = (epoch > MIN_DATETIME);
}

const String getTimeLocal_hhmm() {
    char buf[32];
    sprintf(buf, "%02d:%02d", _time_local.hour, _time_local.minute);
    return String(buf);
}

const String getTimeLocal_hhmmss() {
    char buf[32];
    sprintf(buf, "%02d:%02d:%02d", _time_local.hour, _time_local.minute, _time_local.second);
    return String(buf);
}

const String getDateTimeDotFormated() {
    char buf[32];
    sprintf(buf, "%02d.%02d.%02d %02d:%02d:%02d", _time_local.day_of_month, _time_local.month, _time_local.year, _time_local.hour, _time_local.minute, _time_local.second);
    return String(buf);
}

const String getTodayDateDotFormated() {
    char buf[32];
    sprintf(buf, "%02d.%02d.%d", _time_local.day_of_month, _time_local.month, _time_local.year + 2000);
    return String(buf);
}

// format 22.02.2022
unsigned long strDateToUnix(String date) {
    int day, month, year;
    unsigned int daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    day = date.substring(0, date.indexOf(".")).toInt();
    date = date.substring(date.indexOf(".") + 1);
    month = date.substring(0, date.indexOf(".")).toInt();
    date = date.substring(date.indexOf(".") + 1);
    year = date.toInt();

    unsigned long unixTime = (year - 1970) * 365 * 86400;
    int numberOfLeepYears = (year - 1968) / 4 - (year - 1900) / 100 + (year - 1600) / 400;
    unixTime += numberOfLeepYears * 86400;

    if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) {
        daysInMonth[1] = 29;
    }

    for (int i = 0; i < month - 1; i++) {
        unixTime += daysInMonth[i] * 86400;
    }

    unixTime += (day - 1) * 86400;

    return unixTime;
}

const String getDateTimeDotFormatedFromUnix(unsigned long unixTime) {
    Time_t time;
    breakEpochToTime(unixTime, time);
    char buf[32];
    sprintf(buf, "%02d.%02d.%02d %02d:%02d:%02d", time.day_of_month, time.month, time.year, time.hour, time.minute, time.second);
    return String(buf);
}

const String getTimeDotFormatedFromUnix(unsigned long unixTime) {
    Time_t time;
    breakEpochToTime(unixTime, time);
    char buf[32];
    sprintf(buf, "%02d:%02d:%02d", time.hour, time.minute, time.second);
    return String(buf);
}

const String getDateDotFormatedFromUnix(unsigned long unixTime) {
    Time_t time;
    breakEpochToTime(unixTime, time);
    char buf[32];
    sprintf(buf, "%02d.%02d.%d", time.day_of_month, time.month, time.year + 2000);
    return String(buf);
}
