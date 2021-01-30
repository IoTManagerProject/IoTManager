#ifdef EnableSensorCcs811
#pragma once
#include <Arduino.h>

#include "Adafruit_CCS811.h"
#include "Global.h"
#include "GyverFilters.h"

class SensorCcs811;

typedef std::vector<SensorCcs811> MySensorCcs811Vector;

struct paramsCcs811 {
    String key;
    String addr;
    unsigned long interval;
    float c;
};

class SensorCcs811 {
   public:
    SensorCcs811(const paramsCcs811& paramsPpm, const paramsCcs811& paramsPpb);
    ~SensorCcs811();

    Adafruit_CCS811* ccs811;

    void loop();
    void read();

   private:
    paramsCcs811 _paramsPpm;
    paramsCcs811 _paramsPpb;

    unsigned long prevMillis;
    unsigned long difference;
};

extern MySensorCcs811Vector* mySensorCcs811;

extern void ccs811Sensor();
#endif