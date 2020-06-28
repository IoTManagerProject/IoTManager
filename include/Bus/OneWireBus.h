#pragma once

#include <Arduino.h>

#include <OneWire.h>

class OneWireBus {
   public:
    OneWireBus();
    ~OneWireBus();
    bool exists();
    void set(uint8_t _pin);
    OneWire* get();

   private:
    uint8_t _pin;
    OneWire* _bus;
};

extern OneWireBus oneWireBus;
