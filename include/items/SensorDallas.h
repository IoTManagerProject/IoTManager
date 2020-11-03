#pragma once
#include "Global.h"
#include <Arduino.h>

extern DallasTemperature sensors;
extern OneWire* oneWire;

class SensorDallas;

typedef std::vector<SensorDallas> MySensorDallasVector;

class SensorDallas {
public:

    SensorDallas(unsigned long interval, unsigned int pin, unsigned int index, String key);
    ~SensorDallas();

    void loop();
    void readDallas();

private:

    unsigned long currentMillis;
    unsigned long prevMillis;
    unsigned long _interval;
    String _key;
    unsigned int _pin;
    unsigned int _index;

};

extern MySensorDallasVector* mySensorDallas2;

extern void dallas();


