
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
    Pwm(const String& name) : Item{name},
                              PinAssigned{this},
                              Value{VT_INT},
                              ValueMap{this} {};
    void onAssign() override {
        pinMode(getPin(), OUTPUT);
    }

   protected:
    void onValueUpdate(const String& value) override {
        analogWrite(getPin(), mapValue(value.toInt()));
    }
};

class Pwms {
   public:
    Pwms();
    Pwm* add(String name, String assign, String param);
    Pwm* at(size_t index);
    Pwm* get(const String name);
    size_t count();

   private:
    Pwm* last();

   private:
    std::vector<Pwm*> _items;
};

extern Pwms myPwm;