#ifdef EnableSensorSht20
#pragma once
#include <Arduino.h>
#include "Wire.h"
#include "SHT2x.h"

#include "Global.h"

extern SHT2x* sht;

class SensorSht20;

typedef std::vector<SensorSht20> MySensorSht20Vector;

struct paramsSht {
    String key;
    unsigned long interval;
    float c;
};

class SensorSht20 {
   public:
    SensorSht20(const paramsSht& paramsTmp, const paramsSht& paramsHum);
    ~SensorSht20();

    void loop();
    void read();

   private:
    paramsSht _paramsTmp;
    paramsSht _paramsHum;

    unsigned long prevMillis;
    unsigned long difference;
};

extern MySensorSht20Vector* mySensorSht20;

extern void sht20Sensor();
#endif