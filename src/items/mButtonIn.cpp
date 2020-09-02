#include "items/itemsGlobal.h"
#include "Class/Switch.h"
//==========================================Модуль физических кнопок========================================
//button-in switch1 toggle Кнопки Свет 1 pin[2] db[20]
//==========================================================================================================
void buttonIn() {
    mySwitch = new Switch();
    mySwitch->update();
    String key = mySwitch->gkey();
    String pin = mySwitch->gpin();
    sCmd.addCommand(key.c_str(), buttonInSet);
    mySwitch->init();
    mySwitch->switchStateSetDefault();
    mySwitch->clear();
}

void buttonInSet() {
    String key = sCmd.order();
    String state = sCmd.next();
    mySwitch->switchChangeVirtual(key, state);
}