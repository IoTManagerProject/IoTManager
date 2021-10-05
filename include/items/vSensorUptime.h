#ifdef EnableSensorUptime
#pragma once
#include <Arduino.h>

#include "Global.h"
#include "GyverFilters.h"

class SensorUptime;

typedef std::vector<SensorUptime> MySensorUptimeVector;

struct paramsUptime {
    String key;
    unsigned long interval;
};

class SensorUptime {
   public:
    SensorUptime(const paramsUptime& paramsUpt);
    ~SensorUptime();

    void loop();
    void read();

   private:
    paramsUptime _paramsUpt;

    unsigned long prevMillis;
    unsigned long difference;
};

extern MySensorUptimeVector* mySensorUptime;

extern void uptimeSensor();
#endif