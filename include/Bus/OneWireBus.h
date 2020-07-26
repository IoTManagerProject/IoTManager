#pragma once

#include <Arduino.h>
#include <OneWire.h>

struct OneWireBus_t {
    OneWire *bus;
    uint8_t pin;
};

class OneWireBus {
   public:
    OneWireBus();
    OneWire *get(uint8_t pin);
    size_t count();

   private:
    std::vector<OneWireBus_t> _items;
};

extern OneWireBus oneWireBus;