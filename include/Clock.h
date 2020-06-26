#pragma once

#include "Utils/TimeUtils.h"
#include "Utils/PrintMessage.h"

#include "TZ.h"
#include "sntp.h"

class Clock {
    const char* MODULE = "Clock";

   public:
    Clock() : _timezone{0}, _hasSynced{false}, _configured{false} {}

    void loop() {
        unsigned long passed = millis_since(_uptime);
        if (passed < ONE_SECOND_ms) {
            return;
        }
        _uptime += passed;

        // world time
        time_t now = getSystemTime();
        time_t estimated = _epoch + (passed / ONE_SECOND_ms);
        double drift = difftime(now, estimated);
        if (drift > 1) {
            // Обработать ситуации c дрифтом времени на значительные величины
        }
        // TODO сохранять время на флеше

        _epoch = now;

        breakEpochToTime(_epoch, _time);
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
            pm.info("sync to: " + _ntp + " timezone: " + String(_timezone));
            setupSntp();
            _configured = true;
            // лучше не ждать, проверим в следующий раз
            return;
        }
        _hasSynced = hasTimeSynced();
        if (_hasSynced) {
            pm.info("synced " + getDateDigitalFormated() + " " + getTime());
        } else {
            pm.error("failed to obtain");
        }
    }

    void setupSntp() {
        sntp_setservername(0, _ntp.c_str());
        sntp_setservername(1, "ru.pool.ntp.org");
        sntp_setservername(2, "pool.ntp.org");
        sntp_stop();
        sntp_set_timezone(0);  // UTC time
        sntp_init();
    }

    bool hasTimeSynced() const {
        return _epoch > MIN_DATETIME;
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
        return String(_epoch);
    }

    /*
    * Локальное время "дд.ММ.гг"
    */
    const String getDateDigitalFormated() {
        char buf[16];
        sprintf(buf, "%02d.%02d.%02d", _time.day_of_month, _time.month, _time.year);
        return String(buf);
    }

    /*
    * Локальное время "чч:мм:cc"
    */
    const String getTime() {
        char buf[16];
        sprintf(buf, "%02d:%02d:%02d", _time.hour, _time.minute, _time.second);
        return String(buf);
    }

    /*
    * Локальное время "чч:мм"
    */
    const String getTimeWOsec() {
        char buf[16];
        sprintf(buf, "%02d:%02d", _time.hour, _time.minute);
        return String(buf);
    }

    /*
    * Время с момента запуска "чч:мм" далее "дд чч:мм"
    */
    const String getUptime() {
        return prettyMillis(_uptime);
    }

   private:
    Time_t _time;
    unsigned long _uptime;
    unsigned long _epoch;
    int _timezone;
    String _ntp;
    bool _hasSynced;
    bool _configured;
};