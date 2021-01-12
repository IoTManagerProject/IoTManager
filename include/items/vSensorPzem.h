#pragma once
#include <Arduino.h>

#include "Global.h"
#include "PZEMSensor.h"
#include "SoftUART.h"

class SensorPzem;

typedef std::vector<SensorPzem> MySensorPzemVector;

struct paramsPzem {
    String key;
    String addr;
    unsigned long interval;
    float c;
    float k;
};

class SensorPzem {
   public:
    SensorPzem(const paramsPzem& paramsV, const paramsPzem& paramsA, const paramsPzem& paramsWatt, const paramsPzem& paramsWattHrs, const paramsPzem& paramsHz);
    ~SensorPzem();

    void loop();

   private:
    void read();

    paramsPzem _paramsV;
    paramsPzem _paramsA;
    paramsPzem _paramsWatt;
    paramsPzem _paramsWattHrs;
    paramsPzem _paramsHz;

    PZEMSensor* pzem;

    unsigned long prevMillis;
    unsigned long difference;
};

extern MySensorPzemVector* mySensorPzem;

extern void pzemSensor();