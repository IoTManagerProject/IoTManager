#include "NTP.h"

#include "Global.h"
#include "utils/SerialPrint.h"

void ntpInit() {
    synchTime();

    ts.add(
        TIME, 1000, [&](void*) {
            unixTime = getSystemTime();
            if (unixTime < MIN_DATETIME) {
                isTimeSynch = false;
                // SerialPrint("E", "NTP", "Time not synched");
                jsonWriteInt(errorsHeapJson, F("tme1"), 1);
                synchTime();

                // проверяем присутствие RTC с батарейкой и получаем время при наличии
                if (rtcItem) {
                    unixTime = rtcItem->getRtcUnixTime();
                } else return;
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
    configTime(0, 0, "pool.ntp.org", "ru.pool.ntp.org", jsonReadStr(settingsFlashJson, F("ntp")).c_str());
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
    int day = selectToMarker(date, ".").toInt();
    date = deleteBeforeDelimiter(date, ".");
    int month = selectToMarker(date, ".").toInt();
    date = deleteBeforeDelimiter(date, ".");
    int year = selectToMarker(date, ".").toInt();
    int secsInOneDay = 86400;
    int daysInOneYear = 365;
    int daysInLeepYear = 366;
    int numberOfLeepYears = 12;
    int totalNormalYears = year - 1970 - numberOfLeepYears;
    unsigned int daysInMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int numberOfDaysInPastMonths = 0;
    for (int i = 0; i <= 11; i++) {
        if (i <= month - 2) {
            numberOfDaysInPastMonths = numberOfDaysInPastMonths + daysInMonth[i];
        }
    }
    return (day * secsInOneDay) + (numberOfDaysInPastMonths * secsInOneDay) + (totalNormalYears * daysInOneYear * secsInOneDay) + (numberOfLeepYears * daysInLeepYear * secsInOneDay);
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
