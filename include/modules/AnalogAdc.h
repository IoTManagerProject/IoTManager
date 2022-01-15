#pragma once
#include "Global.h"
#include "Classes/IoTSensor.h"

class AnalogAdc : public IoTSensor {
   public:
    AnalogAdc(String parameters);
    ~AnalogAdc();

    void doByInterval();

   private:
    unsigned int _pin;
};

extern void* getAPI_AnalogAdc(String params);