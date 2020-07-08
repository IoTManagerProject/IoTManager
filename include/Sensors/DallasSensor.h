#pragma once

#include <Arduino.h>

#include <DallasTemperature.h>

#include "OneWireBus.h"

namespace Dallas {
class DallasSensor {
   public:
    DallasSensor(const char* name, OneWireAddress addr) : _addr{addr} {
        strncpy(_name, name, sizeof(_name));
    }

    const char* name() {
        return _name;
    }

    OneWireAddress* addr() {
        return &_addr;
    }

   private:
    OneWireAddress _addr;
    char _name[16];
};


class DallasSensors {
   public:
    DallasSensors();
    DallasSensor* add(String name, OneBusItem item);
    DallasSensor* at(size_t index);
    DallasSensor* get(uint8_t* address);
    DallasSensor* get(String name);
    size_t count();

   private:
    std::vector<DallasSensor> _items;
};

void loop();

extern DallasTemperature* dallasTemperature;
extern DallasSensors dallasSensors;
extern String dallas_value_name;
extern int enter_to_dallas_counter;

}  // namespace Dallas