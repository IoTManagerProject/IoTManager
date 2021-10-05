#ifdef EnableSensorAnalog
#pragma once
#include <Arduino.h>

#include "Global.h"
#include "GyverFilters.h"

class SensorAnalog;

typedef std::vector<SensorAnalog> MySensorAnalogVector;

class SensorAnalog {
   public:
    SensorAnalog(String key, unsigned long interval, unsigned int adcPin, int map1, int map2, int map3, int map4, float c);
    ~SensorAnalog();

    void loop();
    void readAnalog();

   private:
    unsigned long currentMillis;
    unsigned long prevMillis;
    unsigned long difference;

    unsigned long _interval;

    String _key;
    unsigned int _adcPin;

    int _map1;
    int _map2;
    int _map3;
    int _map4;

    float _c;
};

extern MySensorAnalogVector* mySensorAnalog;

extern void analogAdc();
#endif