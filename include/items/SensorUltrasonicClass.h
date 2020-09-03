#pragma once
#include <Arduino.h>
#include "Class/LineParsing.h"
#include "Global.h"
#include "items/SensorConvertingClass.h"

class SensorUltrasonic : public SensorConvertingClass {
   public:
    SensorUltrasonic() : SensorConvertingClass(){};
    void init() {
        sensorReadingMap += _key + ",";
        String trig = selectFromMarkerToMarker(_pin, ",", 0);
        String echo = selectFromMarkerToMarker(_pin, ",", 1);
        jsonWriteStr(configOptionJson, _key + "_trig", trig);
        jsonWriteStr(configOptionJson, _key + "_echo", echo);
        jsonWriteStr(configOptionJson, _key + "_map", _map);
        jsonWriteStr(configOptionJson, _key + "_с", _c);
    }

    void SensorUltrasonicRead(String key) {
        int trig = jsonReadStr(configOptionJson, key + "_trig").toInt();
        int echo = jsonReadStr(configOptionJson, key + "_echo").toInt();
        int value;

        digitalWrite(trig, LOW);
        delayMicroseconds(2);
        digitalWrite(trig, HIGH);
        delayMicroseconds(10);
        digitalWrite(trig, LOW);
        long duration_ = pulseIn(echo, HIGH, 30000);  // 3000 µs = 50cm // 30000 µs = 5 m
        value = duration_ / 29 / 2;

        value = this->mapping(key, value);
        float valueFl = this->correction(key, value);
        eventGen(key, "");
        jsonWriteStr(configLiveJson, key, String(valueFl));
        MqttClient::publishStatus(key, String(valueFl));
        Serial.println("[I] sensor '" + key + "' data: " + String(valueFl));
    }
};
extern SensorUltrasonic mySensorUltrasonic;