#ifdef EnableSensorAny
#pragma once
#include <Arduino.h>

#include "Global.h"

class SensorAny;

typedef std::vector<SensorAny> MySensorAnyVector;

class SensorAny {
   public:
    SensorAny(const String& paramsAny);
    ~SensorAny();

    void loop();
    void read();

   private:
    String _paramsAny;
    int _interval;
    float _c;
    String _key;
    String _addr;
    String _type;

    unsigned long prevMillis;
    unsigned long difference;
};

extern MySensorAnyVector* mySensorAny;

extern void AnySensor();
#endif