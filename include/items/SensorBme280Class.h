//#pragma once
//#include <Arduino.h>
//#include "Class/LineParsing.h"
//#include "Global.h"
//#include "items/SensorConvertingClass.h"
//
//class SensorBme280Class : public SensorConvertingClass {
//   public:
//    SensorBme280Class() : SensorConvertingClass(){};
//
//    void SensorBme280Init() {
//        oneWire = new OneWire((uint8_t)_pin.toInt());
//        sensors.setOneWire(oneWire);
//        sensors.begin();
//        sensors.setResolution(48);
//
//        sensorReadingMap += _key + ",";
//        //Bme280EnterCounter++;
//
//        //jsonWriteInt(configOptionJson, _key + "_num", Bme280EnterCounter);
//        jsonWriteStr(configOptionJson, _key + "_map", _map);
//        jsonWriteStr(configOptionJson, _key + "_с", _c);
//    }
//
//    void SensorBme280Read(String key) {
//        float value;
//        byte num = sensors.getDS18Count();
//        sensors.requestTemperatures();
//
//        int cnt = jsonReadInt(configOptionJson, key + "_num");
//
//        for (byte i = 0; i < num; i++) {
//            if (i == cnt) {
//                value = sensors.getTempCByIndex(i);
//                //value = this->mapping(key, value);
//                float valueFl = this->correction(key, value);
//                eventGen(key, "");
//                jsonWriteStr(configLiveJson, key, String(valueFl));
//                MqttClient::publishStatus(key, String(valueFl));
//                Serial.println("[I] sensor '" + key + "' data: " + String(valueFl));
//            }
//        }
//    }
//};
//extern SensorBme280Class mySensorBme280;