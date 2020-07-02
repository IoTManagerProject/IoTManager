#include "Objects/PwmItems.h"

#include "Utils/PrintMessage.h"

static const char *MODULE = "PWM";

Pwms myPwm;

Pwms::Pwms(){};

Pwm *Pwms::add(String name, String assign, String state) {
    pm.info("name:\"" + name + "\", pin:" + assign + ", state:" + state);
    _items.push_back(Pwm{name, assign, state});
    return last();
}

Pwm *Pwms::last() {
    return &_items.at(_items.size() - 1);
}

Pwm *Pwms::get(const String name) {
    for (size_t i = 0; i < _items.size(); i++) {
        auto *item = &_items.at(i);
        if (name.equalsIgnoreCase(item->name())) {
            return item;
        }
    }
    return nullptr;
}

Pwm *Pwms::at(size_t num) {
    return &_items.at(num);
}

size_t Pwms::count() {
    return _items.size();
}
