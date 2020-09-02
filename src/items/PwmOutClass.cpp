#include "items/itemsGlobal.h"
#include "items/PwmOutClass.h"
//==========================================Модуль управления ШИМ===================================================
//pwm-out volume range Кнопки Свет 1 pin[12] st[500]
//==================================================================================================================
PwmOutClass myPwmOut;
void pwmOut() {
    myPwmOut.update();
    String key = myPwmOut.gkey();
    String pin = myPwmOut.gpin();
    String inv = myPwmOut.ginv();
    sCmd.addCommand(key.c_str(), pwmOutSet);
    jsonWriteStr(configOptionJson, key + "_pin", pin);
    myPwmOut.pwmModeSet();
    myPwmOut.pwmStateSetDefault();
    myPwmOut.clear();
}

void pwmOutSet() {
    String key = sCmd.order();
    String state = sCmd.next();
    String pin = jsonReadStr(configOptionJson, key + "_pin");
    myPwmOut.pwmChange(key, pin, state);
}