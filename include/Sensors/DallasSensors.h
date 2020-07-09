#pragma once

#include <Arduino.h>

#include "Sensors/DallasSensors.h"
#include "Sensors/DallasSensor.h"

namespace Dallas {

class DallasSensors {
   public:
    DallasSensors();
    DallasSensor* add(String name, String addr);
    DallasSensor* add(String name, OneWireAddress addr);
    DallasSensor* at(size_t index);
    DallasSensor* get(uint8_t* address);
    DallasSensor* get(String name);
    size_t count();

    void loop();

   private:
    std::vector<DallasSensor> _items;
};

extern DallasTemperature* dallasTemperature;
extern DallasSensors dallasSensors;

}  // namespace Dallas