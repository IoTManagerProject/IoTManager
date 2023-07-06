#include "utils/TimeUtils.h"
#include "Global.h"
#include "utils/StringUtils.h"

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

const String prettyMinutsTimeout(unsigned long time_m) {
    unsigned long tmp = time_m * 60;
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
        sprintf_P(buf, "%d day %d hr %d min", days, hours, minutes, seconds);
    } else if (hours && !days) {
        sprintf_P(buf, "%d hr %d min", hours, minutes);
    } else {
        sprintf_P(buf, "%d min", minutes);
    }
    return String(buf);
}