#include "Collection/Pwms.h"

#include "PrintMessage.h"

static const char *MODULE = "Pwms";

Pwms pwms;

Pwms::Pwms(){};

Pwm *Pwms::add(String name, String assign) {
    pm.info("name:\"" + name + "\", assign:" + assign);
    Pwm *item = new Pwm{name, assign};
    _items.push_back(item);
    return last();
}

Pwm *Pwms::last() {
    return _items.at(_items.size() - 1);
}

Pwm *Pwms::get(const String name) {
    for (auto *item : _items) {
        if (name.equalsIgnoreCase(item->getName())) {
            return item;
        }
    }
    return NULL;
}

Pwm *Pwms::at(size_t index) {
    return _items.at(index);
}

size_t Pwms::count() {
    return _items.size();
}
