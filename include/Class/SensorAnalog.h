#pragma once
#include <Arduino.h>
#include "Class/LineParsing.h"
#include "Class/SensorConverting.h"
#include "Global.h"

class SensorAnalog : public SensorConverting {
   public:
    SensorAnalog() : SensorConverting(){};

    void SensorAnalogInit(String key) {
    }

    int SensorAnalogRead(String key, String pin) {
        int pinInt = pin.toInt();
        int value;
#ifdef ESP32
        value = analogRead(pinInt);
#endif
#ifdef ESP8266
        pinInt = pinInt;
        value = analogRead(A0);
#endif
        value = this->mapping(key, value);
        float valueFl = this->correction(key, value);

        eventGen(key, "");
        jsonWriteStr(configLiveJson, key, String(valueFl));
        MqttClient::publishStatus(key, String(valueFl));

        Serial.println("[I] sensor '" + key + "' data: " + String(valueFl));
        return value;
    }
};
extern SensorAnalog* mySensorAnalog;