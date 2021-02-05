#include "Consts.h"
#ifdef EnableSensorNode
#include <Arduino.h>

#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Clock.h"
#include "Global.h"
#include "Utils/TimeUtils.h"
#include "items/vSensorNode.h"

SensorNode::SensorNode(const paramsSensorNode& params) {
    _params = paramsSensorNode(params);
    _updateTime = "";
    _minutesPassed = 0;
}

SensorNode::~SensorNode() {}

void SensorNode::loop() {
    difference = millis() - prevMillis;
    if (difference >= 60000) {
        prevMillis = millis();
        _minutesPassed++;
        this->publish();
    }
}

void SensorNode::onChange(String newValue, String incommingKey) {
    if (_params.key == incommingKey) {
        _minutesPassed = 0;
        prevMillis = millis();

        newValue = String(newValue.toFloat() * _params.c);
        newValue = String(newValue.toFloat() + _params.k);

        eventGen2(_params.key, newValue);
        jsonWriteStr(configLiveJson, _params.key, newValue);
        publishStatus(_params.key, newValue);

        _updateTime = timeNow->getDateTimeDotFormated();

        this->publish();
        SerialPrint("I", "Sensor", "'" + _params.key + "' data: " + newValue);
    }
}

void SensorNode::publish() {
    if (_minutesPassed < _params.tm1.toInt()) {
        publishLastUpdateTime(_params.key, String(_minutesPassed) + " min");
        publishAnyJsonKey(_params.key, "", "color");
    } else if (_minutesPassed >= _params.tm1.toInt() && _minutesPassed < _params.tm2.toInt()) {
        publishLastUpdateTime(_params.key, String(_minutesPassed) + " min");
        publishAnyJsonKey(_params.key, "orange", "color");
    } else if (_minutesPassed >= _params.tm2.toInt()) {
        if (_updateTime == "") {
            publishLastUpdateTime(_params.key, "offline");
        } else {
            publishLastUpdateTime(_params.key, _updateTime);
        }
        publishAnyJsonKey(_params.key, "red", "color");
    }
}

MySensorNodeVector* mySensorNode = nullptr;

void nodeSensor() {
    myLineParsing.update();
    String tm1 = myLineParsing.gtm1();
    String tm2 = myLineParsing.gtm2();
    String key = myLineParsing.gkey();
    String c = myLineParsing.gc();
    String k = myLineParsing.gk();
    myLineParsing.clear();

    paramsSensorNode params;

    params.tm1 = tm1;
    params.tm2 = tm2;
    params.key = key;
    params.c = c.toFloat();
    params.k = k.toFloat();

    static bool firstTime = true;
    if (firstTime) mySensorNode = new MySensorNodeVector();
    firstTime = false;
    mySensorNode->push_back(SensorNode(params));
}

void publishTimes() {
    if (mySensorNode != nullptr) {
        for (unsigned int i = 0; i < mySensorNode->size(); i++) {
            mySensorNode->at(i).publish();
        }
    }
}
#endif