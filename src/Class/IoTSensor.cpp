#include "Utils/JsonUtils.h"
#include "Utils/SerialPrint.h"
#include "Class/ScenarioClass3.h"
#include "Class/IoTSensor.h"

void IoTSensor::init(unsigned long interval, String key) {
    _interval = interval * 1000;
    _key = key;
}

IoTSensor::IoTSensor() {}
IoTSensor::~IoTSensor() {}

void IoTSensor::loop() {
    currentMillis = millis();
    difference = currentMillis - prevMillis;
    if (difference >= _interval) {
        prevMillis = millis();
        SerialPrint("I", "Sensor", "Вызывается loop");
        this->doByInterval();
    }
}

void IoTSensor::regEvent(String value, String consoleInfo = "") {
    eventGen2(_key, String(value));
    jsonWriteStr(configLiveJson, _key, String(value));
    publishStatus(_key, String(value));
    SerialPrint("I", "Sensor", "'" + _key + "' data: " + String(value) + "' " + consoleInfo);
}

void IoTSensor::doByInterval() {}