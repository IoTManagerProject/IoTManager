#include "items/itemsGlobal.h"
#include "items/ButtonOutClass.h"
//==========================================Модуль кнопок===================================================
//button-out light toggle Кнопки Свет 1 pin[12] inv[1] st[1]
//==========================================================================================================
ButtonOutClass* myButtonOut;
void buttonOut() {
    myButtonOut = new ButtonOutClass();
    myButtonOut->update();
    String key = myButtonOut->gkey();
    String pin = myButtonOut->gpin();
    String inv = myButtonOut->ginv();
    sCmd.addCommand(key.c_str(), buttonOutSet);
    jsonWriteStr(configOptionJson, key + "_pin", pin);
    jsonWriteStr(configOptionJson, key + "_inv", inv);
    myButtonOut->pinModeSet();
    myButtonOut->pinStateSetDefault();
    myButtonOut->pinStateSetInvDefault();
    myButtonOut->clear();
}

void buttonOutSet() {
    String key = sCmd.order();
    String state = sCmd.next();
    String pin = jsonReadStr(configOptionJson, key + "_pin");
    String inv = jsonReadStr(configOptionJson, key + "_inv");
    if (inv == "") {
        myButtonOut->pinChange(key, pin, state, true);
    } else {
        myButtonOut->pinChange(key, pin, state, false);
    }
}
