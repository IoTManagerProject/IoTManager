#pragma once

#include <Arduino.h>

#include "DallasTemperature.h"
namespace Dallas {
struct DallasSensor_t {
    uint8_t address[8];
    String name;
    DallasSensor_t(uint8_t* address, String name) {
        memcpy(&address, address, sizeof(&address[0]) * 8);
        this->name = name;
    }
};

class DallasSensors {
   public:
    DallasSensors();
    DallasSensor_t* create(uint8_t* address, String name);
    DallasSensor_t* at(size_t index);
    DallasSensor_t* get(uint8_t* address);
    DallasSensor_t* get(String name);
    size_t count();

   private:
    std::vector<DallasSensor_t> _items;
};

void loop();

extern DallasTemperature* dallasTemperature;
extern DallasSensors dallasSensors;
extern String dallas_value_name;
extern int enter_to_dallas_counter;

}