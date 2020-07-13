#pragma once

#include <Arduino.h>

#ifdef ESP8266
#include <Servo.h>
#else
#include <ESP32Servo.h>
#endif

#include "Base/Item.h"

class BaseServo : public Item,
                  public PinAssigned,
                  public Value,
                  public ValueMap {
   public:
    BaseServo(const String& name, const String& assign, const String& value) : Item{name},
                                                                               PinAssigned{this},
                                                                               Value{VT_INT},
                                                                               ValueMap{this} {
        _obj = new Servo();
    };

    void onAssign() override {
        _obj->attach(getPin());
    }

    void onValueUpdate(const String& value) override {
        _obj->write(value.toInt());
    }

   private:
    Servo* _obj;
};
