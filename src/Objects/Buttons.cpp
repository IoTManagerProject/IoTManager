#include "Objects/Buttons.h"

Buttons myButtons;

Buttons::Buttons(){};

Button_t *Buttons::at(size_t index) {
    return &_items.at(index);
}

Bounce *Buttons::create(uint8_t num, uint8_t pin, int delay) {
    // Ищем среди ранее созданных
    for (size_t i = 0; i < _items.size(); i++) {
        auto item = _items.at(i);
        if (item.num == num) {
            if (item.pin != pin) {
                item.obj->attach(pin);
                item.pin = pin;
            };
            item.delay = delay;
            return item.obj;
        }
    }
    // Добавляем новый
    Button_t newItem{num, pin, delay};
    newItem.obj = new Bounce();
    newItem.obj->attach(pin);
    _items.push_back(newItem);
    return newItem.obj;
}

Bounce *Buttons::get(uint8_t num) {
    // Ищем среди ранее созданных
    for (size_t i = 0; i < _items.size(); i++) {
        auto item = _items.at(i);
        if (item.num == num) {
            return item.obj;
        }
    }
    return nullptr;
}

size_t Buttons::count() {
    return _items.size();
}
