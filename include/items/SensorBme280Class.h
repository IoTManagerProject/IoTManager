#pragma once
#include <Arduino.h>

#include "Class/LineParsing.h"
#include "Global.h"
#include "items/SensorConvertingClass.h"

Adafruit_BME280 bme;
Adafruit_Sensor *bme_temp = bme.getTemperatureSensor();
Adafruit_Sensor *bme_pressure = bme.getPressureSensor();
Adafruit_Sensor *bme_humidity = bme.getHumiditySensor();

class SensorBme280Class : public SensorConvertingClass {
   public:
    SensorBme280Class() : SensorConvertingClass(){};

    void SensorBme280Init() {
        bme.begin(hexStringToUint8(_addr));
        jsonWriteStr(configOptionJson, _key + "_map", _map);
        jsonWriteStr(configOptionJson, _key + "_с", _c);
        sensorReadingMap10sec += _key + ",";
    }

    void SensorBme280ReadTmp(String key) {
        float value;
        value = bme.readTemperature();
        float valueFl = this->correction(key, value);
        eventGen(key, "");
        jsonWriteStr(configLiveJson, key, String(valueFl));
         publishStatus(key, String(valueFl));
        SerialPrint("I", "Sensor", "'" + key + "' data: " + String(valueFl));
    }

    void SensorBme280ReadHum(String key) {
        float value;
        value = bme.readHumidity();
        float valueFl = this->correction(key, value);
        eventGen(key, "");
        jsonWriteStr(configLiveJson, key, String(valueFl));
         publishStatus(key, String(valueFl));
        SerialPrint("I", "Sensor", "'" + key + "' data: " + String(valueFl));
    }

    void SensorBme280ReadPress(String key) {
        float value;
        value = bme.readPressure();
        value = value / 1.333224 / 100;
        float valueFl = this->correction(key, value);
        eventGen(key, "");
        jsonWriteStr(configLiveJson, key, String(valueFl));
         publishStatus(key, String(valueFl));
        SerialPrint("I", "Sensor", "'" + key + "' data: " + String(valueFl));
    }
};
extern SensorBme280Class mySensorBme280;