#include "Utils/JsonUtils.h"
#include "Utils/SerialPrint.h"
#include "Classes/ScenarioClass3.h"
#include "Classes/IoTSensor.h"


IoTSensor::IoTSensor(String parameters) {
    _interval = jsonReadInt(parameters, "int") * 1000;
    _subtype = jsonReadStr(parameters, "subtype");
    _id = jsonReadStr(parameters, "id");
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
    eventGen2(_id, String(value));
    jsonWriteStr(paramsFlashJson, _id, String(value));
    publishStatus(_id, String(value));
    SerialPrint("I", "Sensor " + consoleInfo, "'" + _id + "' data: " + String(value) + "'");
}

void IoTSensor::doByInterval() {}

IoTSensor* myIoTSensor;