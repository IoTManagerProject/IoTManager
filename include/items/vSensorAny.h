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
    float _k;
    String _key;
    String _addr;
    String _type;
    String _val;
    String _descr;

    unsigned long prevMillis;
    int difference;
};

extern MySensorAnyVector* mySensorAny;

extern void AnySensor();
#endif