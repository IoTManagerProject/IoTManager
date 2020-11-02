#include "items/SensorDallas.h"
#include "Class/LineParsing.h"
#include "Global.h"
#include "ItemsCmd.h"
#include <Arduino.h>

SensorDallas::SensorDallas(unsigned long period, unsigned int pin, uint8_t deviceAddress, String key) {
    _period = period * 1000;
    _key = key;
    _pin = pin;
    _deviceAddress = deviceAddress;

    oneWire = new OneWire((uint8_t)_pin);
    sensors.setOneWire(oneWire);
    sensors.begin();
    sensors.setResolution(12);
}

SensorDallas::~SensorDallas() {}

void SensorDallas::loop() {
    currentMillis = millis();
    unsigned long difference = currentMillis - prevMillis;
    if (difference >= _period) {
        prevMillis = millis();

    }
}

void readDallas() {

}


MySensorDallasVector* mySensorDallas2 = nullptr;

//void logging() {
//    myLineParsing.update();
//    String loggingValueKey = myLineParsing.gval();
//    String key = myLineParsing.gkey();
//    String interv = myLineParsing.gint();
//    String maxcnt = myLineParsing.gcnt();
//    myLineParsing.clear();
//
//    loggingKeyList += key + ",";
//
//    static bool firstTime = true;
//    if (firstTime) myLogging = new MySensorDallasVector();
//    firstTime = false;
//    myLogging->push_back(SensorDallas(interv.toInt(), maxcnt.toInt(), loggingValueKey, key));
//}

