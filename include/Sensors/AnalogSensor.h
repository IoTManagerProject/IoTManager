#pragma once

#include <Arduino.h>

#include "Base/BaseSensor.h"

class AnalogSensor : public BaseSensor,
                     public ValueMap,
                     public PinAssigned {
   public:
    AnalogSensor(const String& name) : BaseSensor{name, VT_INT}, ValueMap{this}, PinAssigned{this} {};

    void onAssign() override {
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
