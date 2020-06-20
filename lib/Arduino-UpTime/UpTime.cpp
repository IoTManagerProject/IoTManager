/* see UpTime.h */

#include <Arduino.h>
#include <UpTime.h>

unsigned long _uptime_seconds = 0;
void (*time_to_die)(void) = 0; // reset Arduino after 136+ years

unsigned long uptime() {
    unsigned int cur_second = millis() / 1000;
    unsigned int _uptime_seconds_uint = _uptime_seconds;

    while (_uptime_seconds_uint != cur_second) {
        _uptime_seconds++;
        _uptime_seconds_uint++;
        if (_uptime_seconds > 0xFFFF0000) time_to_die();
    }

    return _uptime_seconds;
}

uptime_interval::uptime_interval(unsigned int inte, bool postpone) : interval(inte) {
    reset(postpone);
}

bool uptime_interval::check() {
    if (next <= uptime()) {
        next += interval;
        return true;
    }
    return false;
}

void uptime_interval::reset(bool postpone) {
    next = (postpone ? _uptime_seconds + interval : _uptime_seconds);
}

String _uptime_two_dig(uint8_t x) {
    if (x > 9) {
        return String(x);
    }
    else {
        return "0"+String(x);
    }
}

String uptime_as_string() {
    unsigned long tmp_uptime = uptime();
    unsigned long seconds;
    unsigned long minutes;
    unsigned long hours;
    unsigned long days;
    seconds = tmp_uptime % 60;
    tmp_uptime = tmp_uptime / 60;

    minutes = tmp_uptime % 60;
    tmp_uptime = tmp_uptime / 60;
    hours = tmp_uptime % 24;
    days = tmp_uptime / 24;

    return (days ? String(days)+'d'+' ' : "")
        + _uptime_two_dig(hours)
        + ':' + _uptime_two_dig(minutes)
        + ':' + _uptime_two_dig(seconds);
}
