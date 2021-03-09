#include "Consts.h"
#ifdef EnableButtonOut
#include <Arduino.h>

#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Global.h"
#include "SoftUART.h"
#include "items/vButtonOut.h"
//this class save data to flash
ButtonOut::ButtonOut(String pin, boolean inv, String key, String type) {
    _pin = pin;
    _inv = inv;
    _key = key;
    _type = type;
#ifdef ESP_MODE
    if (_pin != "") {
        pinMode(_pin.toInt(), OUTPUT);
    }
    int state = jsonReadInt(configStoreJson, key); //прочитали из памяти 
    this->execute(String(state)); //установили это состояние
#endif
#ifdef GATE_MODE
//TO DO запросили ноду о состоянии реле
//установили в это состояние кнопку в приложении
//если нода не ответила - кнопку сделать красным цветом
#endif    
}
ButtonOut::~ButtonOut() {}

void ButtonOut::execute(String state) {
#ifdef ESP_MODE
    if (state != "" && _pin != "") {
        if (state == "change") {
            state = String(!digitalRead(_pin.toInt()));
            digitalWrite(_pin.toInt(), state.toInt());
        } else {
            if (_inv) {
                digitalWrite(_pin.toInt(), !state.toInt());
            } else {
                digitalWrite(_pin.toInt(), state.toInt());
            }
        }
    }
#endif
#ifdef GATE_MODE
//отправили ноде команду на вкл выкл
//получили обратную связь - переставили кнопку в приложении
//не получили обратную связь - сделали кнопку красной
#endif
    eventGen2(_key, state);
    jsonWriteInt(configStoreJson, _key, state.toInt());
    saveStore();
    publishStatus(_key, state);
}

MyButtonOutVector* myButtonOut = nullptr;

void buttonOut() {
    myLineParsing.update();
    String key = myLineParsing.gkey();
    String pin = myLineParsing.gpin();
    String inv = myLineParsing.ginv();
    String type = myLineParsing.gtype();

    bool invb = false;
    if (inv.toInt() == 1) invb = true;

    myLineParsing.clear();

    buttonOut_EnterCounter++;
    addKey(key, buttonOut_KeyList, buttonOut_EnterCounter);

    static bool firstTime = true;
    if (firstTime) myButtonOut = new MyButtonOutVector();
    firstTime = false;
    myButtonOut->push_back(ButtonOut(pin, invb, key, type));

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
#endif
