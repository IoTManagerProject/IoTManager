#pragma once

#include <Arduino.h>
#include "Class/LineParsing.h"
#include "Global.h"

class Sensor : public LineParsing {
   public:
    Sensor() : LineParsing(){};

    void SensorInit() {
        if (_pin != "") {
            pinMode(_pin.toInt(), INPUT);
        }
    }

    void SensorRead(String key, String pin, String state) {
        int pinInt = pin.toInt();
        analogWrite(pinInt, state.toInt());
        eventGen(key, "");
        jsonWriteInt(configLiveJson, key, state.toInt());
        MqttClient::publishStatus(key, state);
    }
};

//extern Sensor* mySensor;