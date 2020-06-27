#pragma once

#include <Arduino.h>
#include <OneWire.h>

struct OneWireBus_t {
    OneWire *bus;
    uint8_t pin;
};

class OneWireBus {
   public:
    OneWireBus(){};

    OneWire *get(uint8_t pin) {
        for (size_t i = 0; i < _items.size(); i++) {
            auto item = _items.at(i);
            if (item.pin == pin) {
                return item.bus;
            }
        }
        OneWireBus_t newItem;
        newItem.bus = new OneWire(pin);
        newItem.pin = pin;
        _items.push_back(newItem);
        return newItem.bus;
    }

    size_t count() {
        return _items.size();
    }

   private:
    std::vector<OneWireBus_t> _items;
};
