#include "Objects/Buttons.h"

Buttons myButtons;

Buttons::Buttons(){};

Button_t *Buttons::at(size_t num) {
    return &_items.at(num);
}

Button_t *Buttons::create(String name, uint8_t pin, uint8_t state) {
    Button_t item{name, pin, state};
    _items.push_back(item);
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

size_t Buttons::count() {
    return _items.size();
}
