#pragma once
#include "Global.h"
#include "Classes/IoTSensor.h"

class IoTSensorA : public IoTSensor {
   public:
    IoTSensorA(String parameters);
    ~IoTSensorA();

    void doByInterval();

   private:
    unsigned int _pin;
};

extern IoTSensorA* mySensorAnalog;