#pragma once
#include <Arduino.h>
#include "Class/LineParsing.h"
#include "Global.h"
#include "items/SensorConvertingClass.h"

class SensorDhtClass : public SensorConvertingClass {
   public:
    SensorDhtClass() : SensorConvertingClass(){};

    void SensorDhtInit() {
        //oneWire = new OneWire((uint8_t)_pin.toInt());
        //sensors.setOneWire(oneWire);
        //sensors.begin();
        //sensors.setResolution(48);

        sensorReadingMap += _key + ",";
        //dhtEnterCounter++;

        //jsonWriteInt(configOptionJson, _key + "_num", dhtEnterCounter);
        jsonWriteStr(configOptionJson, _key + "_map", _map);
        jsonWriteStr(configOptionJson, _key + "_с", _c);
    }

    void SensorDhtRead(String key) {
        float value;
        byte num = sensors.getDS18Count();
        sensors.requestTemperatures();

        int cnt = jsonReadInt(configOptionJson, key + "_num");

        for (byte i = 0; i < num; i++) {
            if (i == cnt) {
                value = sensors.getTempCByIndex(i);
                //value = this->mapping(key, value);
                float valueFl = this->correction(key, value);
                eventGen(key, "");
                jsonWriteStr(configLiveJson, key, String(valueFl));
                MqttClient::publishStatus(key, String(valueFl));
                Serial.println("[I] sensor '" + key + "' data: " + String(valueFl));
            }
        }
    }
};
extern SensorDhtClass mySensorDht;