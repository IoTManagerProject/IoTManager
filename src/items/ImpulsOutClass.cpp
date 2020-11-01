#include "items/ImpulsOutClass.h"

#include <Arduino.h>

#include "Class/LineParsing.h"
#include "Global.h"
#include "ItemsCmd.h"

ImpulsOutClass::ImpulsOutClass(unsigned long impulsPeriod, unsigned int impulsCount, unsigned int impulsPin) {
    _impulsPeriod = impulsPeriod;
    _impulsCount = impulsCount * 2;
    _impulsPin = impulsPin;
    pinMode(impulsPin, OUTPUT);
}

ImpulsOutClass::~ImpulsOutClass() {}

void ImpulsOutClass::activate() {
    _impulsCountBuf = _impulsCount;
}

void ImpulsOutClass::loop() {
    currentMillis = millis();
    unsigned long difference = currentMillis - prevMillis;
    if (_impulsCountBuf > 0) {
        if (difference > _impulsPeriod) {
            _impulsCountBuf--;
            prevMillis = millis();
            yield();
            digitalWrite(_impulsPin, !digitalRead(_impulsPin));
            yield();
        }
    }
    if (_impulsCountBuf <= 0) {
        digitalWrite(_impulsPin, LOW);
    }
}

MyImpulsOutVector* myImpulsOut = nullptr;

//void impuls() {
//    myLineParsing.update();
//    String loggingValueKey = myLineParsing.gvalue();
//    String key = myLineParsing.gkey();
//    String interv = myLineParsing.gint();
//    String maxcnt = myLineParsing.gmaxcnt();
//    myLineParsing.clear();
//
//    logging_value_names_list += key + ",";
//
//    static bool firstTime = true;
//    if (firstTime) myImpulsOut = new MyImpulsOutVector();
//    firstTime = false;
//    myImpulsOut->push_back(ImpulsOutClass(interv.toInt(), maxcnt.toInt(), loggingValueKey, key));
//}
