#include "Objects/Timer.h"

#include "Global.h"

Timer::Timer(const char* name, unsigned long time) {
    _name = strdup(name);
    _time = time;
    liveData.write(name, "1", VT_INT);
}

Timer::~Timer() {
    liveData.erase(_name);
    delete _name;
}

bool Timer::tick() {
    if (!_enabled) {
        return false;
    }
    if (!_time--) {
        onTimer();
        return true;
    };
    return false;
}

const char* Timer::name() const {
    return _name;
}

void Timer::onTimer() {
    liveData.write(_name, "0", VT_INT);
    Scenario::fire(_name);
}

void Timer::setTime(unsigned long value) {
    _time = value;
}

void Timer::stop() {
    _enabled = false;
}
