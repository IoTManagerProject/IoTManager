#pragma once

#include <Arduino.h>

#include "Objects/SensorItem.h"
#include "Base/Assigned.h"

class AnalogSensor : public SensorItem,
                     public PinAssigned {
   public:
    AnalogSensor(const String& name, const String& assign) : SensorItem(name, assign),
                                                             PinAssigned{assign} {}

    void onAssign() override {
        pinMode(getPin(), INPUT);
    }

    bool onRead() override {
        _value = analogRead(getPin());
        return true;
    };
};
