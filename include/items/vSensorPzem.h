#pragma once
#include <Arduino.h>

#include "Global.h"
#include "PZEMSensor.h"
#include "SoftUART.h"

extern PZEMSensor* pzem;

class SensorPzem;

typedef std::vector<SensorPzem> MySensorPzemVector;

struct paramsPzem {
    String key;
    String addr;
    unsigned long interval;
    float c;
};

class SensorPzem {
   public:
    SensorPzem(const paramsPzem& paramsV, const paramsPzem& paramsA, const paramsPzem& paramsWatt, const paramsPzem& paramsWattHrs, const paramsPzem& paramsHz);
    ~SensorPzem();

    void loop();
    void read();

   private:
    paramsPzem _paramsV;
    paramsPzem _paramsA;
    paramsPzem _paramsWatt;
    paramsPzem _paramsWattHrs;
    paramsPzem _paramsHz;

    unsigned long prevMillis;
    unsigned long difference;
};

extern MySensorPzemVector* mySensorPzem;

extern void pzemSensor();