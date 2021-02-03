#include "Consts.h"
#ifdef EnableSensorGate
#include <Arduino.h>

#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Global.h"
#include "NodeTimes.h"
#include "items/vSensorGate.h"

SensorGate::SensorGate(const paramsSensorGate& params) {
    _params = paramsSensorGate(params);
    minutes = 0;
}

SensorGate::~SensorGate() {}

void SensorGate::loop() {
    difference = millis() - prevMillis;
    if (difference >= 60000) {
        prevMillis = millis();
        minutes = jsonReadInt(configTimesJson, _params.key);
        minutes++;
        jsonWriteInt(configTimesJson, _params.key, minutes);
        this->setColors(_params.key);
    }
}

void SensorGate::onChange(String newValue, String incommingKey) {
    if (_params.key == incommingKey) {
        minutes = 0;
        prevMillis = millis();
        eventGen2(_params.key, newValue);
        jsonWriteStr(configLiveJson, _params.key, newValue);
        jsonWriteInt(configTimesJson, _params.key, minutes);
        publishStatus(_params.key, newValue);
        this->setColors(_params.key);
        SerialPrint("I", "Sensor", "'" + _params.key + "' data: " + newValue);
    }
}

void SensorGate::setColors(String incommingKey) {
    if (_params.key == incommingKey) {
        if (minutes < _params.orTimeOut.toInt()) {
            publishLastUpdateTime(incommingKey, String(minutes) + " min");
            publishAnyJsonKey(incommingKey, "", "color");
        } else if (minutes >= _params.orTimeOut.toInt() && minutes < _params.rdTimeOut.toInt()) {
            publishLastUpdateTime(incommingKey, String(minutes) + " min");
            publishAnyJsonKey(incommingKey, "orange", "color");
        } else if (minutes >= _params.rdTimeOut.toInt()) {
            publishLastUpdateTime(incommingKey, "offline");
            publishAnyJsonKey(incommingKey, "red", "color");
        }
    }
}

MySensorGateVector* mySensorGate = nullptr;

void gateSensor() {
    myLineParsing.update();
    String orTimeOut = myLineParsing.gtime1();
    String rdTimeOut = myLineParsing.gtime2();
    String key = myLineParsing.gkey();
    String c = myLineParsing.gc();
    String k = myLineParsing.gk();
    myLineParsing.clear();

    paramsSensorGate params;

    params.orTimeOut = orTimeOut;
    params.rdTimeOut = rdTimeOut;
    params.key = key;
    params.c = c.toFloat();
    params.k = k.toFloat();

    static bool firstTime = true;
    if (firstTime) mySensorGate = new MySensorGateVector();
    firstTime = false;
    mySensorGate->push_back(SensorGate(params));
}
#endif