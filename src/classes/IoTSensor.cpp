#include "Utils/JsonUtils.h"
#include "Utils/SerialPrint.h"
#include "Classes/ScenarioClass3.h"
#include "Classes/IoTSensor.h"


IoTSensor::IoTSensor(String parameters) {
    jsonRead(parameters, "int", _interval);
    _interval = _interval * 1000;
    jsonRead(parameters, "subtype", _subtype);
    jsonRead(parameters, "id", _id);
    jsonRead(parameters, "multiply", _multiply);
    jsonRead(parameters, "plus", _plus);
    jsonRead(parameters, "round", _round);
    
    String map;
    jsonRead(parameters, "map", map);
    if (map != "") {
        _map1 = selectFromMarkerToMarker(map, ",", 0).toInt();
        _map2 = selectFromMarkerToMarker(map, ",", 1).toInt();
        _map3 = selectFromMarkerToMarker(map, ",", 2).toInt();
        _map4 = selectFromMarkerToMarker(map, ",", 3).toInt();
    }   
}
IoTSensor::~IoTSensor() {}

String IoTSensor::getSubtype() {
    return _subtype;
}

String IoTSensor::getID() {
    return _id;
};

void IoTSensor::loop() {
    currentMillis = millis();
    difference = currentMillis - prevMillis;
    if (difference >= _interval) {
        prevMillis = millis();
        this->doByInterval();
    }
}

void IoTSensor::regEvent(String value, String consoleInfo = "") {
    eventGen2(_id, value);
    jsonWriteStr(paramsFlashJson, _id, value);
    publishStatus(_id, value);
    SerialPrint("I", "Sensor " + consoleInfo, "'" + _id + "' data: " + value + "'");
}

void IoTSensor::regEvent(float value, String consoleInfo = "") {  
    if (_multiply) value = value * _multiply;
    if (_plus) value = value + _multiply;
    if (_round != 0) {    
        if (value > 0) {
            value = (int)(value * _round + 0.5F);
            value = value / _round;
        }
        if (value < 0) {
            value = (int)(value * _round - 0.5F);
            value = value / _round;
        }
    }
    if (_map1 != _map2) value = map(value, _map1, _map2, _map3, _map4);

    // убираем лишние нули
    char buf[20];
    sprintf(buf, "%g", value);
    regEvent((String)buf, consoleInfo);
}

void IoTSensor::doByInterval() {}

IoTSensor* myIoTSensor;