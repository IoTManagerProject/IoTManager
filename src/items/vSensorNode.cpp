#include "Consts.h"
#ifdef EnableSensorNode
#include <Arduino.h>

#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Global.h"
#include "NodeTimes.h"
#include "items/vSensorNode.h"

SensorNode::SensorNode(const paramsSensorNode& params) {
    _params = paramsSensorNode(params);
    minutes = 0;
}

SensorNode::~SensorNode() {}

void SensorNode::loop() {
    difference = millis() - prevMillis;
    if (difference >= 60000) {
        prevMillis = millis();
        minutes = jsonReadInt(configTimesJson, _params.key);
        minutes++;
        jsonWriteInt(configTimesJson, _params.key, minutes);
        this->setColors(_params.key);
    }
}

void SensorNode::onChange(String newValue, String incommingKey) {
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

void SensorNode::setColors(String incommingKey) {
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

MySensorNodeVector* mySensorNode = nullptr;

void nodeSensor() {
    myLineParsing.update();
    String orTimeOut = myLineParsing.gtime1();
    String rdTimeOut = myLineParsing.gtime2();
    String key = myLineParsing.gkey();
    String c = myLineParsing.gc();
    String k = myLineParsing.gk();
    myLineParsing.clear();

    paramsSensorNode params;

    params.orTimeOut = orTimeOut;
    params.rdTimeOut = rdTimeOut;
    params.key = key;
    params.c = c.toFloat();
    params.k = k.toFloat();

    static bool firstTime = true;
    if (firstTime) mySensorNode = new MySensorNodeVector();
    firstTime = false;
    mySensorNode->push_back(SensorNode(params));
}
#endif