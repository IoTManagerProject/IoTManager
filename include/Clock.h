#pragma once

#include "Clock.h"
#include "Global.h"
#include "Utils/TimeUtils.h"
#include "Utils\SerialPrint.h"

extern void clockInit();

#ifdef ESP8266
#include "sntp.h"
#endif



class Clock {
   private:
    Time_t _time_local;
    Time_t _time_utc;
    unsigned long _uptime;
    unsigned long _unixtime;
    int _timezone;
    String _ntp;
    bool _hasSynced;
    bool _configured;

   public:
    Clock() : _uptime{0}, _timezone{0}, _ntp{""}, _hasSynced{false}, _configured{false} {};

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

        _unixtime = now;

        breakEpochToTime(_unixtime, _time_utc);

        breakEpochToTime(_unixtime + getOffsetInSeconds(_timezone), _time_local);
    }

    bool hasSync() {
        if (!_hasSynced) {
            startSync();
        }
        return _hasSynced;
    }

    void setNtpPool(String ntp) {
        if (!_ntp.equals(ntp)) {
            _ntp = ntp;
            _configured = false;
        }
    }

    void setTimezone(int timezone) {
        if (_timezone != timezone) {
            _timezone = timezone;
            _configured = false;
        }
    }

    void startSync() {
        if (!_configured) {
            SerialPrint("I", "NTP", "sync to: " + _ntp + " timezone: " + String(_timezone));
            setupSntp();
            _configured = true;
            return;
        }
        _hasSynced = hasTimeSynced();
        if (_hasSynced) {
            SerialPrint("I", "NTP", "synced " + getDateDotFormated() + " " + getTime());
        } else {
            SerialPrint("E", "NTP", F("failed to obtain time"));
        }
    }

    void setupSntp() {
#ifdef ESP8266
        sntp_setservername(0, _ntp.c_str());
        sntp_setservername(1, "ru.pool.ntp.org");
        sntp_setservername(2, "pool.ntp.org");
        sntp_stop();
        sntp_set_timezone(0);  
        sntp_init();
#else
        configTime(0, 0, _ntp.c_str(), "ru.pool.ntp.org", "pool.ntp.org");
#endif
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
    * Локальное дата время "дд.ММ.гг чч.мм.cc"
    */
    const String getDateTimeDotFormated(Time_t timeNow) {
        char buf[32];
        sprintf(buf, "%02d.%02d.%02d %02d:%02d:%02d", timeNow.day_of_month, timeNow.month, timeNow.year, timeNow.hour, timeNow.minute, timeNow.second);
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
};
extern Clock* timeNow;