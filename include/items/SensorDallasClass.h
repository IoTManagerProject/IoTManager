#pragma once
#include <Arduino.h>
#include "Class/LineParsing.h"
#include "Global.h"
#include "items/SensorConvertingClass.h"

extern DallasTemperature sensors;
extern OneWire* oneWire;
class SensorDallasClass : public SensorConvertingClass {
public:
    SensorDallasClass() : SensorConvertingClass() {};

    void SensorDallasInit() {
        oneWire = new OneWire((uint8_t)_pin.toInt());
        sensors.setOneWire(oneWire);
        sensors.begin();
        sensors.setResolution(48);

        sensorReadingMap10sec += _key + ",";
        dallasEnterCounter++;

        jsonWriteInt(configOptionJson, _key + "_num", dallasEnterCounter);
        jsonWriteStr(configOptionJson, _key + "_map", _map);
        jsonWriteStr(configOptionJson, _key + "_с", _c);
    }

    void SensorDallasRead(String key) {
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
                publishStatus(key, String(valueFl));
                SerialPrint("I", "Sensor", "'" + key + "' data: " + String(valueFl));
            }
        }
    }
};
extern SensorDallasClass mySensorDallas;