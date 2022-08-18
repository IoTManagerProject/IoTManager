#include "NTP.h"

#include "Global.h"
#include "Utils\SerialPrint.h"

void ntpInit() {
    synchTime();

    ts.add(
        TIME, 1000, [&](void*) {
            unsigned long unixTime = getSystemTime();
            if (unixTime < MIN_DATETIME) {
                // SerialPrint("E", "NTP", "Time not synched");
                synchTime();
                return;
            }
            breakEpochToTime(unixTime + jsonReadInt(settingsFlashJson, F("timezone")) * 60 * 60, _time_local);
            breakEpochToTime(unixTime, _time_utc);
            String timenow = getTimeLocal_hhmm();
            static String prevTime;
            if (prevTime != timenow) {
                prevTime = timenow;
                String dateAndTime = getDateTimeDotFormated();
                dateAndTime = deleteToMarkerLast(dateAndTime, ":");
                jsonWriteStr_(errorsHeapJson, F("timenow"), dateAndTime);
                SerialPrint("I", F("NTP"), "✔ " + dateAndTime);
            }
        },
        nullptr, true);

    SerialPrint("I", F("NTP"), F("Handle time init"));
}

void synchTime() {
    configTime(0, 0, "pool.ntp.org", "ru.pool.ntp.org", "pool.ntp.org");
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
