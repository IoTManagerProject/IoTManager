#ifdef EnableSensorGate
#pragma once
#include <Arduino.h>

#include "Global.h"

class SensorGate;

typedef std::vector<SensorGate> MySensorGateVector;

struct paramsSensorGate {
    String orTimeOut;
    String rdTimeOut;
    String key;
    float c;
    float k;
};

class SensorGate {
   public:
    SensorGate(const paramsSensorGate& params);
    ~SensorGate();

    int minutes;

    void loop();
    void onChange(String newValue, String incommingKey);
    void setColors(String incommingKey);

   private:
    paramsSensorGate _params;

    unsigned long prevMillis;
    unsigned long difference;
};

extern MySensorGateVector* mySensorGate;

extern void gateSensor();
#endif