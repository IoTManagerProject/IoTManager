#pragma once

#include <Arduino.h>

#ifdef ESP8266
#include <Servo.h>
#else
#include <ESP32Servo.h>
#endif

#include "Base/Item.h"
#include "Base/ValueMap.h"

class BaseServo : public Item {
   public:
    BaseServo(const String& name, const String& pin, const String& value, Mapper* map) : Item{name, pin, value},
                                                                                         _mapper{map} {
        _obj = new Servo();
        _obj->write(_mapper->evaluate(_state));
        Serial.println("new name: " + String(_name) + ", state: " + String(_state, DEC));
    };

    void onAssign(const char* value) {
        _pin = atoi(value);
        _obj->attach(_pin);
    }

    void onStateChange() override {
        Serial.println("change state name: " + String(_name) + ", state: " + String(_state, DEC) + "mapped: " + String(_mapper->evaluate(_state), DEC));
        _obj->write(_mapper->evaluate(_state));
    }

   private:
    uint8_t _pin;
    Mapper* _mapper;
    Servo* _obj;
};
