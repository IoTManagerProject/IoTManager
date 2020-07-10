#pragma once

#include <Arduino.h>

#include "Objects/SensorItem.h"
#include "Base/Assigned.h"

class AnalogSensorItem : public SensorItem, public PinAssigned {
   public:
    AnalogSensorItem(const String& name, const String& assign) : SensorItem(name, assign), PinAssigned{assign} {}

    void onPinAssign() override {
        pinMode(getPin(), INPUT);
    }

    int onRead() override {
#ifdef ESP32
        int analogRead(34);
#endif
        return analogRead(getPin());
    };
};

class AnalogSensors {
   public:
    AnalogSensorItem* add(const String& name, const String& pin);
    void update();
    AnalogSensorItem* last();

   private:
    std::vector<AnalogSensorItem> _items;
};

extern AnalogSensors analogSensor;
