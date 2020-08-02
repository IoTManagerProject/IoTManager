#pragma once

#include <Arduino.h>

#include "Class/LineParsing.h"
#include "Global.h"

class Input : public LineParsing {
   public:
    Input() : LineParsing(){};

    void inputSetDefault() {
        inputSet(_key, _state);
    }

    void inputSet(String key, String state) {
        eventGen(key, "");
        jsonWriteFloat(configLiveJson, key, state.toFloat());
        MqttClient::publishStatus(key, state);
    }
};

extern Input* myInput;