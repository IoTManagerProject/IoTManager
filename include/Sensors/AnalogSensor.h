#pragma once

#include <Arduino.h>

#include "Base/BaseSensor.h"

class AnalogSensor : public BaseSensor,
                     public PinAssigned,
                     public ValueMap {
   public:
    AnalogSensor(const String& name, const String& assign) : BaseSensor{name, assign, VT_INT},
                                                             PinAssigned{this},
                                                             ValueMap{this} {
        pinMode(getPin(), INPUT);
    }

    const bool hasValue() override {
        return true;
    }

    const String onGetValue() override {
        int raw = onReadSensor().toInt();
        int mapped = mapValue(raw);
        return String(mapped, DEC);
    }

    const String onReadSensor() override {
        return String(analogRead(getPin()), DEC);
    }
};
