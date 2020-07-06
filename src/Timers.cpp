#include "Timers.h"

#include "Events.h"
#include "Global.h"

Timer::Timer(const char* name, unsigned long time) {
    strncpy(_name, name, sizeof(_name));
    _time = time;
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
    String name = "timer";
    name += _name;
    
    liveData.writeInt(name, 0);
    Events::fire(name);
}

void Timer::setTime(unsigned long value) {
    _time = value;
}

void Timer::stop() {
    _enabled = false;
}
namespace Timers {
std::vector<Timer*> _items;

void loop() {
    for (size_t i = 0; i < _items.size(); i++) {
        Timer* item = _items.at(i);
        if (item->tick()) {
            _items.erase(_items.begin() + i);
        }
    }
}

Timer* add(const String& name, unsigned long time) {
    for (size_t i = 0; i < _items.size(); i++) {
        Timer* item = _items.at(i);
        if (name.equalsIgnoreCase(item->name())) {
            item->setTime(time);
            return item;
        }
    }
    _items.push_back(new Timer{name.c_str(), time});
    return _items.at(_items.size() - 1);
}

void erase(const String& name) {
    for (size_t i = 0; i < _items.size(); i++) {
        Timer* item = _items.at(i);
        if (name.equalsIgnoreCase(item->name())) {
            _items.erase(_items.begin() + i);
            break;
        }
    }
}

}  // namespace Timers