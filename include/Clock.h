#pragma once

#include "Utils/TimeUtils.h"
#include "Utils/PrintMessage.h"

#include "time.h"

class Clock {
    const char* MODULE = "Clock";

   public:
    Clock() : _timezone{0}, _ntp{}, _hasSynced{false}, _configured{false} {
    }

    bool hasSync() {
        if (!_hasSynced) {
            startSync();
        }
        return _hasSynced;
    }

    void

    setNtpPool(String ntp) {
        _ntp = ntp;
    }

    void setTimezone(int timezone) {
        _timezone = timezone;
    }

    time_t getSystemTime() {
        timeval tv{0, 0};
        timezone tz = getTimeZone(getBiasInMinutes());
        time_t epoch = 0;
        if (gettimeofday(&tv, &tz) != -1)
            epoch = tv.tv_sec;
        return epoch;
    }

    void startSync() {
        if (!_configured) {
            pm.info("sync to: " + _ntp + " time zone: " + String(_timezone));
            setupSntp();
            _configured = true;
        }
        _hasSynced = hasTimeSynced();
        if (_hasSynced) {
            pm.info("synced " + getDateDigitalFormated() + " " + getTime());
        } else {
            pm.error("failed to obtain");
        }
    }

    void setupSntp() {
        int tzs = getBiasInSeconds();
        int tzh = tzs / 3600;
        tzs -= tzh * 3600;
        int tzm = tzs / 60;
        tzs -= tzm * 60;

        char tzstr[64];
        snprintf(tzstr, sizeof tzstr, "ESPUSER<%+d:%02d:%02d>", tzh, tzm, tzs);
        pm.info(String(tzstr));
        configTime(tzstr, _ntp.c_str());
    }
    // #ifdef ESP32
    //     uint8_t i = 0;
    //     struct tm timeinfo;
    //     while (!getLocalTime(&timeinfo) && i <= 4) {
    //         Serial.print(".");
    //         i++;
    //         delay(1000);
    //     }
    // #endif
   private:
    bool hasTimeSynced() {
        unsigned long now = time(nullptr);
        return now > millis();
    }

    int getBiasInSeconds() {
        return getBiasInMinutes() * 60;
    }

    int getBiasInMinutes() {
        return _timezone * 60;
    }

    const timezone getTimeZone(int minutes) {
        return timezone{minutes, 0};
    }

   private:
    int _timezone;
    String _ntp;
    bool _hasSynced;
    bool _configured;
};