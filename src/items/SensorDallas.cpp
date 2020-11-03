#include "items/SensorDallas.h"
#include "Class/LineParsing.h"
#include "Global.h"
#include "ItemsCmd.h"
#include <Arduino.h>

SensorDallas::SensorDallas(unsigned long interval, unsigned int pin, unsigned int index, String key) {
    _interval = interval * 1000;
    _key = key;
    _pin = pin;
    _index = index;

    oneWire = new OneWire((uint8_t)_pin);
    sensors.setOneWire(oneWire);
    sensors.begin();
    sensors.setResolution(12);
}

SensorDallas::~SensorDallas() {}

void SensorDallas::loop() {
    currentMillis = millis();
    unsigned long difference = currentMillis - prevMillis;
    if (difference >= _interval) {
        prevMillis = millis();
        readDallas();
    }
}

void SensorDallas::readDallas() {
    sensors.requestTemperaturesByIndex(_index);
    float value = sensors.getTempCByIndex(_index);
    eventGen(_key, "");
    jsonWriteStr(configLiveJson, _key, String(value));
    publishStatus(_key, String(value));
    SerialPrint("I", "Sensor", "'" + _key + "' data: " + String(value));
}

MySensorDallasVector* mySensorDallas2 = nullptr;

void dallas() {
    myLineParsing.update();
    String interval = myLineParsing.gint();
    String pin = myLineParsing.gpin();
    String index = myLineParsing.gindex();
    String key = myLineParsing.gkey();
    myLineParsing.clear();

    static bool firstTime = true;
    if (firstTime) mySensorDallas2 = new MySensorDallasVector();
    firstTime = false;
    mySensorDallas2->push_back(SensorDallas(interval.toInt(), pin.toInt(), index.toInt(), key));
}

