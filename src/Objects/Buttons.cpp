#include "Objects/Buttons.h"

#include "Utils/PrintMessage.h"
#include <functional>

static const char *MODULE = "button";

Buttons myButtons;

Buttons::Buttons(){};

Button_t *Buttons::addSwitch(String name, String assign, String param) {
    pm.info("name:\"" + name + "\", pin:" + assign + ", debounce:" + param);
    _items.push_back(Button_t{BUTTON_SWITCH, name, assign, param});
    return last();
}

Button_t *Buttons::addButton(String name, String assign, String param) {
    pm.info("name:\"" + name + "\", pin:" + assign + ", state:" + param);
    _items.push_back(Button_t{BUTTON_PINNED, name, assign, param});
    return last();
}

Button_t *Buttons::last() {
    return &_items.at(_items.size() - 1);
}

Button_t *Buttons::get(const String name) {
    for (size_t i = 0; i < _items.size(); i++) {
        auto item = _items.at(i);
        if (item.name == name) {
            return &_items.at(i);
        }
    }
    return nullptr;
}

Button_t *Buttons::at(size_t num) {
    return &_items.at(num);
}

size_t Buttons::count() {
    return _items.size();
}

void Buttons::loop() {
    for (size_t i = 0; i < _items.size(); i++) {
        Button_t *btn = &_items.at(i);
        if (btn->update()) {
            onChangeState(btn, i);
        }
    }
}

void Buttons::setOnChangeState(OnButtonChangeState h) {
    onChangeState = h;
}
