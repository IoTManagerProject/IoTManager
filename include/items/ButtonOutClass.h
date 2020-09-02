#pragma once
#include <Arduino.h>
#include "Class/LineParsing.h"
#include "Global.h"

class ButtonOutClass : public LineParsing {
   public:
    ButtonOutClass() : LineParsing(){};

    void pinModeSet() {
        if (_pin != "") {
            pinMode(_pin.toInt(), OUTPUT);
        }
    }

    void pinStateSetDefault() {
        if (_inv == "" && _state != "") {
            pinChange(_key, _pin, _state, true);
        }
    }

    void pinStateSetInvDefault() {
        if (_inv != "" && _state != "") {
            pinChange(_key, _pin, _state, false);
        }
    }

    void pinChange(String key, String pin, String state, bool rev) {
        int pinInt = pin.toInt();
        int stateInt;
        if (rev) {
            digitalWrite(pinInt, state.toInt());
        } else {
            digitalWrite(pinInt, !state.toInt());
        }
        eventGen(key, "");
        jsonWriteInt(configLiveJson, key, state.toInt());
        MqttClient::publishStatus(key, state);
    }
};

extern ButtonOutClass myButtonOut;