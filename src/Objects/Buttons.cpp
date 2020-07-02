#include "Objects/Buttons.h"

#include "Utils/PrintMessage.h"
#include <functional>

static const char* MODULE = "button";

Buttons myButtons;

Button* Buttons::add(String name, String assign, String value) {
    pm.info("name:\"" + name + "\", pin:" + assign + ", state:" + value);
    _items.push_back(Button{name, assign, value});
    return last();
}

Button* Buttons::last() {
    return &_items.at(_items.size() - 1);
}

Button* Buttons::get(const String name) {
    for (size_t i = 0; i < _items.size(); i++) {
        auto* item = &_items.at(i);
        if (name.equals(item->getName())) {
            return &_items.at(i);
        }
    }
    return nullptr;
}

Button* Buttons::at(size_t num) {
    return &_items.at(num);
}

size_t Buttons::count() {
    return _items.size();
}

void Buttons::loop() {
    for (size_t i = 0; i < _items.size(); i++) {
        Button* btn = &_items.at(i);
        if (btn->update()) {
            _onChange(btn, i);
        }
    }
}

void Buttons::setOnChangeState(OnButtonChangeState h) {
    _onChange = h;
}
