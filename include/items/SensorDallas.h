#pragma once
#include "Global.h"
#include <Arduino.h>

extern DallasTemperature sensors;
extern OneWire* oneWire;

class SensorDallas;

typedef std::vector<SensorDallas> MySensorDallasVector;

class SensorDallas {
   public:

    SensorDallas(unsigned long period,unsigned int pin, uint8_t deviceAddress, String key);
    ~SensorDallas();

    void loop();

   private:

    unsigned long currentMillis;
    unsigned long prevMillis;
    unsigned long _period;
    String _key;
    unsigned int _pin;
    uint8_t _deviceAddress;

    void readDallas();

};

extern MySensorDallasVector* mySensorDallas2;


