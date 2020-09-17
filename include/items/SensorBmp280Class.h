#pragma once
#include <Arduino.h>

#include "Class/LineParsing.h"
#include "Global.h"
#include "items/SensorConvertingClass.h"

Adafruit_BMP280 bmp;
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();

class SensorBmp280Class : public SensorConvertingClass {
   public:
    SensorBmp280Class() : SensorConvertingClass(){};

    void SensorBmp280Init() {
        bmp.begin(hexStringToUint8(_addr));
        jsonWriteStr(configOptionJson, _key + "_map", _map);
        jsonWriteStr(configOptionJson, _key + "_с", _c);
        sensorReadingMap += _key + ",";
    }

    void SensorBmp280ReadTmp(String key) {
        float value;
        sensors_event_t temp_event;
        bmp_temp->getEvent(&temp_event);
        value = temp_event.temperature;
        float valueFl = this->correction(key, value);
        eventGen(key, "");
        jsonWriteStr(configLiveJson, key, String(valueFl));
        MqttClient::publishStatus(key, String(valueFl));
        Serial.println("I sensor '" + key + "' data: " + String(valueFl));
    }

    void SensorBmp280ReadPress(String key) {
        float value;
        sensors_event_t pressure_event;
        bmp_pressure->getEvent(&pressure_event);
        value = pressure_event.pressure;
        value = value / 1.333224;
        float valueFl = this->correction(key, value);
        eventGen(key, "");
        jsonWriteStr(configLiveJson, key, String(valueFl));
        MqttClient::publishStatus(key, String(valueFl));
        Serial.println("I sensor '" + key + "' data: " + String(valueFl));
    }
};
extern SensorBmp280Class mySensorBmp280;