
#pragma once

#include <Arduino.h>

#ifdef ESP8266
#include <Servo.h>
#else
#include <ESP32Servo.h>
#endif

#include "Base/Item.h"
#include "Base/Assigned.h"

class Pwm : public Item,
            public PinAssigned,
            public Value,
            public ValueMap {
   public:
    Pwm(const String& name, const String& assign) : Item{name, assign},
                                                    PinAssigned{this},
                                                    Value{VT_INT},
                                                    ValueMap{this} {
        pinMode(getPin(), OUTPUT);
    }

   protected:
    void onValueUpdate(const String& value) override {
        analogWrite(getPin(), mapValue(value.toInt()));
    }
};
