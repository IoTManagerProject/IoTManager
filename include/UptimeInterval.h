#pragma once

#include <Arduino.h>

class UptimeInterval {
   public:
    UptimeInterval(unsigned long interval, boolean postpone = true) : _interval{interval} {
        reset(postpone);
    }

    boolean check() {
        if (_next <= get()) {
            _next += _interval;
            return true;
        }
        return false;
    }

    void reset(bool postpone = true) {
        _next = (postpone ? _uptime_seconds + _interval : _uptime_seconds);
    }

    static unsigned long get() {
        unsigned long _uptime_seconds = millis() / 1000;
        return _uptime_seconds;
    };

    static unsigned long _uptime_seconds;

   private:
    unsigned long _interval, _next;
};
