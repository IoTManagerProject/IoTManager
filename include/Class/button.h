#pragma once

#include <Arduino.h>

#include "Class/Item.h"
#include "Global.h"

class Button  {
   public:

    void pinModeSet(int pinf) {
        if (pin != "") {
            pinMode(pinf, OUTPUT);
        }
    }

    void pinStateSet(int pinf, int statef) {
        if (state != "") {
            digitalWrite(pinf, statef);
            jsonWriteInt(configLiveJson, key, statef);
            MqttClient::publishStatus(key, String(statef));
        }
    }

    void pinStateSetInv(int pinf, int statef) {
        if (inv != "" && state != "") {
            digitalWrite(pinf, !statef);
            jsonWriteInt(configLiveJson, key, !statef);
            MqttClient::publishStatus(key, String(!statef));
        }
    }
};

extern Button* myButton;