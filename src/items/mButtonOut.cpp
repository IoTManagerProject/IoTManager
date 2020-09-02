#include "items/itemsGlobal.h"
#include "Class/Button.h"
//==========================================Модуль кнопок===================================================
//button-out light toggle Кнопки Свет 1 pin[12] inv[1] st[1]
//==========================================================================================================
void buttonOut() {
    myButton = new Button1();
    myButton->update();
    String key = myButton->gkey();
    String pin = myButton->gpin();
    String inv = myButton->ginv();
    sCmd.addCommand(key.c_str(), buttonOutSet);
    jsonWriteStr(configOptionJson, key + "_pin", pin);
    jsonWriteStr(configOptionJson, key + "_inv", inv);
    myButton->pinModeSet();
    myButton->pinStateSetDefault();
    myButton->pinStateSetInvDefault();
    myButton->clear();
}

void buttonOutSet() {
    String key = sCmd.order();
    String state = sCmd.next();
    String pin = jsonReadStr(configOptionJson, key + "_pin");
    String inv = jsonReadStr(configOptionJson, key + "_inv");
    if (inv == "") {
        myButton->pinChange(key, pin, state, true);
    } else {
        myButton->pinChange(key, pin, state, false);
    }
}
