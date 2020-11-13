#include "items/ButtonOut.h"

#include <Arduino.h>

#include "Class/LineParsing.h"
#include "Global.h"
#include "BufferExecute.h"

ButtonOut::ButtonOut(unsigned int pin, boolean inv, String key) {
    _pin = pin;
    _inv = inv;
    _key = key;
}
ButtonOut::~ButtonOut() {}

void ButtonOut::init() {
    pinMode(_pin, OUTPUT);
}

void ButtonOut::execute(String state) {
    //if (_inv) {
    //    digitalWrite(_pin, !state.toInt());
    //}
    //else {
        digitalWrite(_pin, state.toInt());
    //}
    eventGen2(_key, state);
    jsonWriteInt(configLiveJson, _key, state.toInt());
    publishStatus(_key, state);
}

MyButtonOutVector* myButtonOut = nullptr;

void buttonOut() {
    myLineParsing.update();
    String key = myLineParsing.gkey();
    String pin = myLineParsing.gpin();
    String inv = myLineParsing.ginv();
    myLineParsing.clear();

    buttonOut_EnterCounter++;
    addKey(key, buttonOut_KeyList, buttonOut_EnterCounter);

    static bool firstTime = true;
    if (firstTime) myButtonOut = new MyButtonOutVector();
    firstTime = false;
    myButtonOut->push_back(ButtonOut(pin.toInt(), inv.toInt(), key));

    sCmd.addCommand(key.c_str(), buttonOutExecute);
}

void buttonOutExecute() {
    String key = sCmd.order();
    String state = sCmd.next();

    int number = getKeyNum(key, buttonOut_KeyList);

    if (myButtonOut != nullptr) {
        if (number != -1) {
            myButtonOut->at(number).execute(state);
        }
    }
}

