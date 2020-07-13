#include "Objects/Switches.h"

#include "Utils/PrintMessage.h"

static const char* MODULE = "Switch";

Switches mySwitches;

void Switches::setOnChangeState(OnSwitchChangeState h) {
    _onChange = h;
}

Switch* Switches::add(const String& name, const String& assign) {
    pm.info("name: \"" + name + "\", pin: " + assign);
    if (!isDigitStr(assign)) {
        pm.error("wrong pin");
        return nullptr;
    }
    Switch* item = new Switch{name};
    item->setAssign(assign);
    _items.push_back(item);
    return last();
}

Switch* Switches::last() {
    return _items.at(_items.size() - 1);
}

Switch* Switches::get(const String name) {
    for (size_t i = 0; i < _items.size(); i++) {
        auto* item = _items.at(i);
        if (name.equals(item->getName())) {
            return _items.at(i);
        }
    }
    return nullptr;
}

Switch* Switches::at(size_t num) {
    return _items.at(num);
}

size_t Switches::count() {
    return _items.size();
}

void Switches::loop() {
    for (size_t i = 0; i < _items.size(); i++) {
        Switch* item = _items.at(i);
        if (item->hasValue()) {
            item->getValue();
        }
    }
}
