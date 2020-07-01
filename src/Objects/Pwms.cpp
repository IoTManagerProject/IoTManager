#include "Objects/Pwms.h"

#include "Utils/PrintMessage.h"

static const char *MODULE = "pwm";

Pwms myPwms;

Pwms::Pwms(){};

PwmItem *Pwms::add(String name, String assign, String state) {
    pm.info("name:\"" + name + "\", pin:" + assign + ", state:" + state);
    _items.push_back(PwmItem{name, assign, state});
    return last();
}

PwmItem *Pwms::last() {
    return &_items.at(_items.size() - 1);
}

PwmItem *Pwms::get(const String name) {
    for (size_t i = 0; i < _items.size(); i++) {
        auto *item = &_items.at(i);
        if (name.equalsIgnoreCase(item->name())) {
            return item;
        }
    }
    return nullptr;
}

PwmItem *Pwms::at(size_t num) {
    return &_items.at(num);
}

size_t Pwms::count() {
    return _items.size();
}
