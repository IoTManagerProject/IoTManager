#include "Consts.h"
#ifdef EnablePwmOut
#include "items/vPwmOut.h"

#include <Arduino.h>

#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Consts.h"
#include "Global.h"

//this class save data to flash
PwmOut::PwmOut(unsigned int pin, String key) {
    _pin = pin;
    _key = key;
    pinMode(_pin, OUTPUT);
    int state = jsonReadInt(configStoreJson, key);
    this->execute(String(state));
}
PwmOut::~PwmOut() {}

void PwmOut::execute(String state) {
    analogWrite(_pin, state.toInt());
    eventGen2(_key, state);
    jsonWriteInt(configStoreJson, _key, state.toInt());
    saveStore();
    publishStatus(_key, state);
     String path = mqttRootDevice + "/" + _key + "/status";
    String json = "{}";
    jsonWriteStr(json, "status", state);
    String MyJson = json; 
    jsonWriteStr(MyJson, "topic", path); 
    
    #ifdef WEBSOCKET_ENABLED
    ws.textAll(MyJson);
    #endif
}

MyPwmOutVector* myPwmOut = nullptr;

void pwmOut() {
    myLineParsing.update();
    String key = myLineParsing.gkey();
    String pin = myLineParsing.gpin();
    myLineParsing.clear();

    pwmOut_EnterCounter++;
    addKey(key, pwmOut_KeyList, pwmOut_EnterCounter);

    static bool firstTime = true;
    if (firstTime) myPwmOut = new MyPwmOutVector();
    firstTime = false;
    myPwmOut->push_back(PwmOut(pin.toInt(), key));

    sCmd.addCommand(key.c_str(), pwmOutExecute);
}

void pwmOutExecute() {
    String key = sCmd.order();
    String state = sCmd.next();

    int number = getKeyNum(key, pwmOut_KeyList);

    if (myPwmOut != nullptr) {
        if (number != -1) {
            myPwmOut->at(number).execute(state);
        }
    }
}
#endif