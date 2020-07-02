#pragma once

#include <Arduino.h>
#include "Base/Item.h"
#include "Base/ValueMapper.h"

class BaseServo : public Item {
   public:
    BaseServo(const String& name, const String& pin, const String& value, ValueMapper* mapper) : Item{name, pin, value},
                                                                                                 _mapper{mapper} {
        _obj = new Servo();
        _obj->attach(getPin());
        _obj->write(_mapper->getValue(_state));
        Serial.println("new name: " + String(_name) + ", state: " + String(_state, DEC));
    };

    void onStateChange() override {
        Serial.println("change state name: " + String(_name) + ", state: " + String(_state, DEC) + "mapped: " + String(_mapper->getValue(_state), DEC));
        _obj->write(_mapper->getValue(_state));
    }

   private:
    ValueMapper* _mapper;
    Servo* _obj;
};
