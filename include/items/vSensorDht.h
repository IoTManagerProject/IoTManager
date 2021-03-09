#ifdef EnableSensorDht
#pragma once
#include <Arduino.h>
#include <DHTesp.h>

#include "Global.h"
#include "GyverFilters.h"

class SensorDht;

typedef std::vector<SensorDht> MySensorDhtVector;

struct paramsDht {
    String type;
    String key;
    unsigned long interval;
    unsigned int pin;
    float c;
};

class SensorDht {
   public:
    SensorDht(const paramsDht& paramsTmp, const paramsDht& paramsHum);
    ~SensorDht();

    DHTesp* dht;

    void loop();
    void readTmpHum();

   private:
    paramsDht _paramsTmp;
    paramsDht _paramsHum;

    unsigned long prevMillis;
    unsigned long difference;
};

extern MySensorDhtVector* mySensorDht;

extern void dhtSensor();
#endif