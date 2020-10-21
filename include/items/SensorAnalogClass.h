#pragma once
#include <Arduino.h>

#include "Class/LineParsing.h"
#include "Global.h"
#include "items/SensorConvertingClass.h"

class SensorAnalogClass : public SensorConvertingClass {
   public:
    SensorAnalogClass() : SensorConvertingClass(){};

    void SensorAnalogInit() {
        jsonWriteStr(configOptionJson, _key + "_pin", _pin);
        jsonWriteStr(configOptionJson, _key + "_map", _map);
        jsonWriteStr(configOptionJson, _key + "_с", _c);
    }

    int SensorAnalogRead(String key, String pin) {
        int value;
#ifdef ESP32
        int pinInt = pin.toInt();
        value = analogRead(pinInt);
#endif
#ifdef ESP8266
        value = analogRead(A0);
#endif
        value = this->mapping(key, value);
        float valueFl = this->correction(key, value);
        eventGen(key, "");
        jsonWriteStr(configLiveJson, key, String(valueFl));
        publishStatus(key, String(valueFl));
        SerialPrint("I", "Sensor", "'" + key + "' data: " + String(valueFl));
        return value;
    }
};
extern SensorAnalogClass mySensorAnalog;