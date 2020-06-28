#include "Objects/Buttons.h"

#include "Utils/PrintMessage.h"
#include <functional>

static const char *MODULE = "button";

Buttons myButtons;

Buttons::Buttons(){};

Button_t *Buttons::create(String name, uint8_t pin, uint8_t state) {
    pm.info("name:\"" + name + "\" pin:" + pin + " state:" + state);
    Button_t item{name, pin, state};
    _items.push_back(item);
    return &_items.at(_items.size() - 1);
}

Bounce *Buttons::obj(size_t num) {
    return _items.at(num).obj;
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

