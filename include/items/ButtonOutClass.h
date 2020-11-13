//#pragma once
//#include <Arduino.h>
//
//#include "Class/LineParsing.h"
//#include "Global.h"
//
//class ButtonOutClass : public LineParsing {
//public:
//    ButtonOutClass() : LineParsing() {};
//
//    void init() {
//        if (_pin != "") {
//            pinMode(_pin.toInt(), OUTPUT);
//        }
//        jsonWriteStr(configOptionJson, _key + "_pin", _pin);
//        jsonWriteStr(configOptionJson, _key + "_inv", _inv);
//    }
//
//    void pinStateSetDefault() {
//        pinChange(_key, _state);
//    }
//
//
//    void pinChange(String key, String state) {
//        String pin = jsonReadStr(configOptionJson, key + "_pin");
//        String inv = jsonReadStr(configOptionJson, key + "_inv");
//        int pinInt = pin.toInt();
//
//        if (inv == "") {
//            digitalWrite(pinInt, state.toInt());
//        }
//        else {
//            digitalWrite(pinInt, !state.toInt());
//        }
//        eventGen2(key, state);
//        jsonWriteInt(configLiveJson, key, state.toInt());
//        publishStatus(key, state);
//    }
//};
//
//extern ButtonOutClass myButtonOut;