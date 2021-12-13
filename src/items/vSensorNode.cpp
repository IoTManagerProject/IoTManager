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
           String path = mqttRootDevice + "/" +_params.key + "/status";
    String json = "{}";
    jsonWriteStr(json, "status", newValue);
    String MyJson = json; 
    jsonWriteStr(MyJson, "topic", path); 
    ws.textAll(MyJson);

        _updateTime = timeNow->getDateTimeDotFormated();

        this->publish();
        //SerialPrint("I", "Sensor", "'" + _params.key + "' data: " + newValue);
    }
}

void SensorNode::publish() {
    if (_minutesPassed < _params.tm1.toInt()) {
        publishAnyJsonKey(_params.key, "info", String(_minutesPassed) + " min");
    
    String path = mqttRootDevice + "/" + _params.key + "/status";
    String json = "{}";
    jsonWriteStr(json, "info", String(_minutesPassed) + " min");
     String MyJson = json; 
        jsonWriteStr(MyJson, "topic", path); 
        ws.textAll(MyJson);

        
        publishAnyJsonKey(_params.key, "color", "");
        
        
     path = mqttRootDevice + "/" + _params.key + "/status";
     json = "{}";
    jsonWriteStr(json, "color", "");
      MyJson = json; 
        jsonWriteStr(MyJson, "topic", path); 
        ws.textAll(MyJson);

    } else if (_minutesPassed >= _params.tm1.toInt() && _minutesPassed < _params.tm2.toInt()) {
        publishAnyJsonKey(_params.key, "info", String(_minutesPassed) + " min");
       
       String   path = mqttRootDevice + "/" + _params.key + "/status";
     String json = "{}";
    jsonWriteStr(json, "info", String(_minutesPassed) + " min");
      String MyJson = json; 
        jsonWriteStr(MyJson, "topic", path); 
        ws.textAll(MyJson);

        publishAnyJsonKey(_params.key, "color", "orange");
                  path = mqttRootDevice + "/" + _params.key + "/status";
     json = "{}";
    jsonWriteStr(json, "color", "orange");
      MyJson = json; 
        jsonWriteStr(MyJson, "topic", path); 
        ws.textAll(MyJson);

    } else if (_minutesPassed >= _params.tm2.toInt()) {
        if (_updateTime == "") {
            publishAnyJsonKey(_params.key, "info", "offline");
         
         
     String path = mqttRootDevice + "/" + _params.key + "/status";
    String json = "{}";
    jsonWriteStr(json, "info", "offline");
     String MyJson = json; 
        jsonWriteStr(MyJson, "topic", path); 
        ws.textAll(MyJson);
        } else {
            publishAnyJsonKey(_params.key, "info", _updateTime);
     String path = mqttRootDevice + "/" + _params.key + "/status";
    String json = "{}";
    jsonWriteStr(json, "info", _updateTime);
     String MyJson = json; 
        jsonWriteStr(MyJson, "topic", path); 
        ws.textAll(MyJson);
     
     
        }
        publishAnyJsonKey(_params.key, "color", "red");
       String path = mqttRootDevice + "/" + _params.key + "/status";
    String json = "{}";
    jsonWriteStr(json, "color", "red");
     String MyJson = json; 
        jsonWriteStr(MyJson, "topic", path); 
        ws.textAll(MyJson);
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