#include "Bus/OneWireBus.h"

OneWireBus oneWireBus;

OneWireBus::OneWireBus(){};

OneWire *OneWireBus::get(uint8_t pin) {
    // Ищем среди ранее созданных
    for (size_t i = 0; i < _items.size(); i++) {
        auto item = _items.at(i);
        if (item.pin == pin) {
            return item.bus;
        }
    }
    // Добавляем новый
    OneWireBus_t newItem;
    newItem.bus = new OneWire(pin);
    newItem.pin = pin;
    _items.push_back(newItem);
    return newItem.bus;
}

size_t OneWireBus::count() {
    return _items.size();
}
