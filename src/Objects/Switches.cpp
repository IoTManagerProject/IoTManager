#include "Objects/Switches.h"

#include "Utils/PrintMessage.h"

static const char* MODULE = "Switch";

Switches mySwitches;

Switch* Switches::add(const String& name, const String& assign, const String& value) {
    if (!isDigitStr(assign)) {
        pm.error("wrong pin");
        return nullptr;
    }
    if (!isDigitStr(value)) {
        pm.error("wrong debounce");
        return nullptr;
    }
    uint16_t debounce = value.toInt();
    uint8_t pin = assign.toInt();
    _items.push_back(Switch{name, pin, debounce});
    return last();
}

Switch* Switches::last() {
    return &_items.at(_items.size() - 1);
}

Switch* Switches::get(const String name) {
    for (size_t i = 0; i < _items.size(); i++) {
        auto* item = &_items.at(i);
        if (name.equals(item->getName())) {
            return &_items.at(i);
        }
    }
    return nullptr;
}

Switch* Switches::at(size_t num) {
    return &_items.at(num);
}

size_t Switches::count() {
    return _items.size();
}

void Switches::loop() {
    for (size_t i = 0; i < _items.size(); i++) {
        Switch* item = &_items.at(i);
        if (item->update()) {
            _onChange(item, i);
        }
    }
}

void Switches::setOnChangeState(OnSwitchChangeState h) {
    _onChange = h;
}
