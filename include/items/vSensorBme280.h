#ifdef EnableSensorBme280
#pragma once
#include <Adafruit_BME280.h>
#include <Arduino.h>

#include "Global.h"

extern Adafruit_BME280* bme;

class SensorBme280;

typedef std::vector<SensorBme280> MySensorBme280Vector;

struct paramsBme {
    String key;
    String addr;
    unsigned long interval;
    float c;
};

class SensorBme280 {
   public:
    SensorBme280(const paramsBme& paramsTmp, const paramsBme& paramsHum, const paramsBme& paramsPrs);
    ~SensorBme280();

    void loop();
    void read();

   private:
    paramsBme _paramsTmp;
    paramsBme _paramsHum;
    paramsBme _paramsPrs;

    unsigned long prevMillis;
    unsigned long difference;
};

extern MySensorBme280Vector* mySensorBme280;

extern void bme280Sensor();
#endif