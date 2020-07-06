#pragma once

#include <Arduino.h>

#include "Config/ClockConfig.h"
#include "Utils/TimeUtils.h"
#include "Utils/PrintMessage.h"

#ifdef ESP8266
#include "sntp.h"
#endif

struct Time_t {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day_of_week;
    uint8_t day_of_month;
    uint8_t month;
    uint16_t day_of_year;
    uint16_t year;
    unsigned long days;
    unsigned long valid;
};

class Clock {
    const char* MODULE = "Clock";

   private:
    Time_t _time_local;
    Time_t _time_utc;
    unsigned long _uptime;
    unsigned long _unixtime;
    bool _hasSynced;
    bool _configured;
    ClockConfig* _cfg;
    const uint8_t DAYS_IN_MONTH[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

   public:
    Clock() : _uptime{millis()}, _hasSynced{false}, _configured{false} {};

    void setConfig(ClockConfig* cfg) {
        _cfg = cfg;
    }

    void loop() {
        unsigned long passed = millis_since(_uptime);
        if (passed < ONE_SECOND_ms) {
            return;
        }
        _uptime += passed;

        // world time
        time_t now = getSystemTime();
        time_t estimated = _unixtime + (passed / ONE_SECOND_ms);
        double drift = difftime(now, estimated);
        if (drift > 1) {
            // Обработать ситуации c дрифтом времени на значительные величины
        }
        // TODO сохранять время на флеше

        _unixtime = now;

        breakEpochToTime(_unixtime, _time_utc);

        breakEpochToTime(_unixtime + getOffsetInSeconds(_cfg->getTimezone()), _time_local);
    }

    bool hasSync() {
        if (!_hasSynced) {
            startSync();
        }
        return _hasSynced;
    }

    void startSync() {
        if (!_configured) {
            pm.info("ntp: " + _cfg->getNtp() + " timezone: " + String(_cfg->getTimezone()));
            setupSntp();
            _configured = true;
            // лучше не ждать, проверим в следующий раз
            return;
        }
        _hasSynced = hasTimeSynced();
        if (_hasSynced) {
            pm.info(getDateDotFormated() + " " + getTime());
        } else {
            pm.error("failed to obtain");
        }
    }

    void setupSntp() {
        configTime(0, 0, _cfg->getNtp().c_str(), "ru.pool.ntp.org", "pool.ntp.org");
    }

    bool hasTimeSynced() const {
        return _unixtime > MIN_DATETIME;
    }

    time_t getSystemTime() const {
        timeval tv{0, 0};
        timezone tz = timezone{0, 0};
        time_t epoch = 0;
        if (gettimeofday(&tv, &tz) != -1) {
            epoch = tv.tv_sec;
        }
        return epoch;
    }
    const unsigned long getEpoch() {
        return _unixtime;
    }
    const String getTimeUnix() {
        return String(_unixtime);
    }

    /*
    * Локальное время "дд.ММ.гг"
    */
    const String getDateDotFormated() {
        char buf[32];
        sprintf(buf, "%02d.%02d.%02d", _time_local.day_of_month, _time_local.month, _time_local.year);
        return String(buf);
    }

    /*
    * Локальное дата время "дд.ММ.гг чч.мм.cc"
    */
    const String getDateTimeDotFormated() {
        char buf[32];
        sprintf(buf, "%02d.%02d.%02d %02d:%02d:%02d", _time_local.day_of_month, _time_local.month, _time_local.year, _time_local.hour, _time_local.minute, _time_local.second);
        return String(buf);
    }

    /*
    * Локальное время "чч:мм:cc"
    */
    const String getTime() {
        char buf[32];
        sprintf(buf, "%02d:%02d:%02d", _time_local.hour, _time_local.minute, _time_local.second);
        return String(buf);
    }

    const String getTimeJson() {
        char buf[32];
        sprintf(buf, "%02d-%02d-%02d", _time_local.hour, _time_local.minute, _time_local.second);
        return String(buf);
    }

    /*
    * Локальное время "чч:мм"
    */
    const String getTimeWOsec() {
        char buf[32];
        sprintf(buf, "%02d:%02d", _time_local.hour, _time_local.minute);
        return String(buf);
    }

    /*
    * Время с момента запуска "чч:мм:cc" далее "дд чч:мм"
    */
    const String getUptime() {
        return prettyMillis(_uptime);
    }

    /*
    * Разбивает время на составляющие
    */
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
                month_length = DAYS_IN_MONTH[month];
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
};