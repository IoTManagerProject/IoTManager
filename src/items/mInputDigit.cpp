#include "items/itemsGlobal.h"
#include "Class/Input.h"
//==========================================Модуль ввода цифровых значений==================================
//input-digit digit1 inputDigit Ввод Введите.цифру 4 st[60]
//==========================================================================================================
void inputDigit() {
    myInput = new Input();
    myInput->update();
    String key = myInput->gkey();
    sCmd.addCommand(key.c_str(), inputDigitSet);
    myInput->inputSetDefaultFloat();
    myInput->clear();
}

void inputDigitSet() {
    String key = sCmd.order();
    String state = sCmd.next();
    myInput->inputSetFloat(key, state);
}