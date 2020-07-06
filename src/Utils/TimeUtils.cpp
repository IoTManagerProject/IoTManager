#include "Utils\TimeUtils.h"
#include "Utils\StringUtils.h"


static const char* WEEK_DAYS[7] = {"Sun", "Mon", "Tue", "Wed", "Thr", "Fri", "Sat"};
static const char* TIME_FORMAT PROGMEM = "%02d:%02d:%02d";
static const char* TIME_FORMAT_WITH_DAYS PROGMEM = "%dd %02d:%02d";

const String prettySeconds(unsigned long time_s) {
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

    char buf[32];

    if (days) {
        sprintf_P(buf, TIME_FORMAT_WITH_DAYS, days, hours, minutes, seconds);
    } else {
        sprintf_P(buf, TIME_FORMAT, hours, minutes, seconds);
    }
    return String(buf);
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
