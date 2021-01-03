#pragma once
#include <Adafruit_BMP280.h>
#include <Arduino.h>

#include "Global.h"

extern Adafruit_BMP280* bmp;

class SensorBmp280;

typedef std::vector<SensorBmp280> MySensorBmp280Vector;

struct paramsBmp {
    String key;
    String addr;
    unsigned long interval;
    float c;
};

class SensorBmp280 {
   public:
    SensorBmp280(const paramsBmp& paramsTmp, const paramsBmp& paramsPrs);
    ~SensorBmp280();

    void loop();
    void read();

   private:
    paramsBmp _paramsTmp;
    paramsBmp _paramsPrs;

    unsigned long prevMillis;
    unsigned long difference;
};

extern MySensorBmp280Vector* mySensorBmp280;

extern void bmp280Sensor();