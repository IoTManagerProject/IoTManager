#pragma once
#include <Arduino.h>
#include "Class/LineParsing.h"
#include "Global.h"

class PwmOutClass : public LineParsing {
   public:
    PwmOutClass() : LineParsing(){};

    void pwmModeSet() {
        if (_pin != "") {
            pinMode(_pin.toInt(), INPUT);
        }
    }

    void pwmStateSetDefault() {
        if (_state != "") {
            pwmChange(_key, _pin, _state);
        }
    }

    void pwmChange(String key, String pin, String state) {
        int pinInt = pin.toInt();
        analogWrite(pinInt, state.toInt());
        eventGen2(key, state);
        jsonWriteInt(configLiveJson, key, state.toInt());
         publishStatus(key, state);
    }
};

extern PwmOutClass myPwmOut;