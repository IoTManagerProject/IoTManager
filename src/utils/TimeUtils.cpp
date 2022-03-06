#include "Utils/TimeUtils.h"

static const uint8_t days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static const char* TIME_FORMAT PROGMEM = "%02d:%02d:%02d";
static const char* TIME_FORMAT_WITH_DAYS PROGMEM = "%dd %02d:%02d";

char* prettySeconds(unsigned long time_s, char* buf, size_t buf_size) {
    unsigned long tmp = time_s;
    unsigned long seconds;
    unsigned long minutes;
    unsigned long hours;
    unsigned long days;
    seconds = tmp % 60;
    tmp = tmp / 60;

    minutes = tmp % 60;
    tmp = tmp / 60;

    hours = tmp % 24;
    days = tmp / 24;

    if (days) 
        snprintf_P(buf, buf_size, TIME_FORMAT_WITH_DAYS, days, hours, minutes, seconds);
     else 
        snprintf_P(buf, buf_size, TIME_FORMAT, hours, minutes, seconds);
    return buf;
}

const String prettySeconds(unsigned long time_s) {
    char buf[32];
    return prettySeconds(time_s, buf, sizeof(buf));
}

const String prettyMillis(unsigned long time_ms) {
    return prettySeconds(time_ms / 1000);
}

unsigned long millis_since(unsigned long sinse) {
    return millis_passed(sinse, millis());
}

unsigned long millis_passed(unsigned long start, unsigned long finish) {
    unsigned long result = 0;
    if (start <= finish) {
        unsigned long passed = finish - start;
        if (passed <= __LONG_MAX__) {
            result = static_cast<long>(passed);
        } else {
            result = static_cast<long>((__LONG_MAX__ - finish) + start + 1u);
        }
    } else {
        unsigned long passed = start - finish;
        if (passed <= __LONG_MAX__) {
            result = static_cast<long>(passed);
            result = -1 * result;
        } else {
            result = static_cast<long>((__LONG_MAX__ - start) + finish + 1u);
            result = -1 * result;
        }
    }
    return result;
}

int getOffsetInSeconds(int timezone) {
    return getOffsetInMinutes(timezone) * ONE_MINUTE_s;
}

int getOffsetInMinutes(int timezone) {
    return timezone * ONE_HOUR_m;
}

void breakEpochToTime(unsigned long epoch, Time_t& tm) {
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
    tm.month = month + 1;
    tm.day_of_month = time + 1;
    tm.valid = (epoch > MIN_DATETIME);
}

// void timeInit() {
//     ts.add(
//         TIME, 1000, [&](void*) {
//             String timenow = timeNow->getTimeWOsec();
//             static String prevTime;
//             if (prevTime != timenow) {
//                 prevTime = timenow;
//                 jsonWriteStr(paramsFlashJson, "timenow", timenow);
//                 eventGen2("timenow", timenow);
//             SerialPrint('I', F("NTP"), timenow);
//             }
//         },
//         nullptr, true);
// SerialPrint('I', F("NTP"), F("Handle time init"));
// }
