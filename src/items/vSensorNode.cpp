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
    minutes = 0;
    jsonWriteInt(configTimesJson, _params.key, minutes);
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

        newValue = String(newValue.toFloat() * _params.c);
        newValue = String(newValue.toFloat() + _params.k);

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
        if (minutes < _params.tm1.toInt()) {
            publishLastUpdateTime(incommingKey, String(minutes) + " min");
            publishAnyJsonKey(incommingKey, "", "color");
        } else if (minutes >= _params.tm1.toInt() && minutes < _params.tm2.toInt()) {
            publishLastUpdateTime(incommingKey, String(minutes) + " min");
            publishAnyJsonKey(incommingKey, "orange", "color");
        } else if (minutes >= _params.tm2.toInt()) {
            unsigned long unix = timeNow->getTimeUnix().toInt();
            unix = unix - (minutes * 60);
            Time_t lastTime;
            breakEpochToTime(unix, lastTime);
            publishLastUpdateTime(incommingKey, timeNow->getDateTimeDotFormated(lastTime));
            publishAnyJsonKey(incommingKey, "red", "color");
        }
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
    if (configTimesJson != "{}") {
        String str = configTimesJson;
        str.replace("{", "");
        str.replace("}", "");
        str.replace("\"", "");
        str += ",";
        while (str.length() != 0) {
            String tmp = selectToMarker(str, ",");
            String key = selectToMarker(tmp, ":");
            String minutes = deleteBeforeDelimiter(tmp, ":");
            if (key != "" && minutes != "") {
                if (mySensorNode != nullptr) {
                    for (unsigned int i = 0; i < mySensorNode->size(); i++) {
                        mySensorNode->at(i).setColors(key);
                    }
                }
            }
            str = deleteBeforeDelimiter(str, ",");
        }
    }
}
#endif