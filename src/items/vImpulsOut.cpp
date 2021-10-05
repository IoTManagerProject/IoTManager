#include "Consts.h"
#ifdef EnableImpulsOut
#include "items/vImpulsOut.h"
#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Global.h"
#include "BufferExecute.h"
#include <Arduino.h>

ImpulsOutClass::ImpulsOutClass(unsigned int impulsPin) {
    _impulsPin = impulsPin;
    pinMode(impulsPin, OUTPUT);
}

ImpulsOutClass::~ImpulsOutClass() {}

void ImpulsOutClass::execute(unsigned long impulsPeriod, unsigned int impulsCount) {
    _impulsPeriod = impulsPeriod;
    _impulsCount = impulsCount * 2;
    _impulsCountBuf = _impulsCount;
}

void ImpulsOutClass::loop() {
    currentMillis = millis();
    difference = currentMillis - prevMillis;
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

void impuls() {
    myLineParsing.update();
    String key = myLineParsing.gkey();
    String pin = myLineParsing.gpin();
    myLineParsing.clear();

    impuls_EnterCounter++;
    addKey(key, impuls_KeyList, impuls_EnterCounter);

    static bool firstTime = true;
    if (firstTime) myImpulsOut = new MyImpulsOutVector();
    firstTime = false;
    myImpulsOut->push_back(ImpulsOutClass(pin.toInt()));

    sCmd.addCommand(key.c_str(), impulsExecute);
}

void impulsExecute() {
    String key = sCmd.order();
    String impulsPeriod = sCmd.next();
    String impulsCount = sCmd.next();

    int number = getKeyNum(key, impuls_KeyList);

    if (myImpulsOut != nullptr) {
        if (number != -1) {
            myImpulsOut->at(number).execute(impulsPeriod.toInt(), impulsCount.toInt());
        }
    }
}

#endif
