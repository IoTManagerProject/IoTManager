#pragma once
#include <Arduino.h>
#include <DHTesp.h>

#include "Global.h"
#include "GyverFilters.h"

extern DHTesp* dht;

class SensorDht;

typedef std::vector<SensorDht> MySensorDhtVector;

struct params {
    String type;
    String key;
    unsigned long interval;
    unsigned int pin;
    float c;
};

class SensorDht {
   public:
    SensorDht(const params& paramsTmp, const params& paramsHum);
    ~SensorDht();

    void loop();
    void readTmpHum();

   private:
    params _paramsTmp;
    params _paramsHum;
    
    unsigned int _interval;

    unsigned long prevMillis;
    unsigned long difference;
};

extern MySensorDhtVector* mySensorDht;

extern void dhtSensor();