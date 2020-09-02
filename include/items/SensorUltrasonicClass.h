#pragma once
#include <Arduino.h>

#include "Class/LineParsing.h"
#include "items/SensorConvertingClass.h"
#include "Global.h"

class SensorUltrasonic : public SensorConvertingClass {
   public:
    SensorUltrasonic() : SensorConvertingClass(){};

    int SensorUltrasonicRead(String key, String pin) {
        int trig = selectFromMarkerToMarker(pin, ",", 0).toInt();
        int echo = selectFromMarkerToMarker(pin, ",", 1).toInt();
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
        return value;
    }
};
extern SensorUltrasonic* mySensorUltrasonic;