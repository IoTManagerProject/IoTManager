#include "items/itemsGlobal.h"
#include "items/ButtonInClass.h"
//==========================================Модуль физических кнопок========================================
//button-in switch1 toggle Кнопки Свет 1 pin[2] db[20]
//==========================================================================================================
ButtonInClass myButtonIn;
void buttonIn() {
    myButtonIn.update();
    String key = myButtonIn.gkey();
    String pin = myButtonIn.gpin();
    sCmd.addCommand(key.c_str(), buttonInSet);
    myButtonIn.init();
    myButtonIn.switchStateSetDefault();
    myButtonIn.clear();
}

void buttonInSet() {
    String key = sCmd.order();
    String state = sCmd.next();
    myButtonIn.switchChangeVirtual(key, state);
}