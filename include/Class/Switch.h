#pragma once

#include <Arduino.h>
#include "Class/LineParsing.h"
#include "Global.h"

class Switch : public LineParsing {
   public:
    Switch() : LineParsing(){};

    void switchModeSet() {
        if (_pin != "") {
            pinMode(_pin.toInt(), INPUT);
        }
    }

    void switchStateSetDefault() {
        if (_inv == "" && _state != "") {
            switchChange(_key, _pin, _state, true);
        }
    }

    void switchStateSetInvDefault() {
        if (_inv != "" && _state != "") {
            switchChange(_key, _pin, _state, false);
        }
    }

    void switchChange(String key, String pin, String state, bool rev) {
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

extern Switch* mySwitch;