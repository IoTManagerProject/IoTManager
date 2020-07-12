#pragma once

#include <Arduino.h>

#include "Base/BaseSensor.h"

class AnalogSensor : public BaseSensor {
   public:
    AnalogSensor(const String& name, const String& assign) : BaseSensor(name, assign){};

    void onAssign() override {
        Serial.printf("pin: %d" + getPin());
        pinMode(getPin(), INPUT);
    }

    int onReadSensor() override {
        return analogRead(getPin());
    }
};
