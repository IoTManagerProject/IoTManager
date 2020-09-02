#include "items/itemsGlobal.h"
#include "Class/Pwm.h"
//==========================================Модуль управления ШИМ===================================================
//pwm-out volume range Кнопки Свет 1 pin[12] st[500]
//==================================================================================================================
void pwmOut() {
    myPwm = new Pwm();
    myPwm->update();
    String key = myPwm->gkey();
    String pin = myPwm->gpin();
    String inv = myPwm->ginv();
    sCmd.addCommand(key.c_str(), pwmOutSet);
    jsonWriteStr(configOptionJson, key + "_pin", pin);
    myPwm->pwmModeSet();
    myPwm->pwmStateSetDefault();
    myPwm->clear();
}

void pwmOutSet() {
    String key = sCmd.order();
    String state = sCmd.next();
    String pin = jsonReadStr(configOptionJson, key + "_pin");
    myPwm->pwmChange(key, pin, state);
}