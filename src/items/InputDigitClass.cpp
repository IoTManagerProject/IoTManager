#include "items/itemsGlobal.h"
#include "items/InputClass.h"
//==========================================Модуль ввода цифровых значений==================================
//input-digit digit1 inputDigit Ввод Введите.цифру 4 st[60]
//==========================================================================================================
InputClass* myInputDigit;
void inputDigit() {
    myInputDigit = new InputClass();
    myInputDigit->update();
    String key = myInputDigit->gkey();
    sCmd.addCommand(key.c_str(), inputDigitSet);
    myInputDigit->inputSetDefaultFloat();
    myInputDigit->clear();
}

void inputDigitSet() {
    String key = sCmd.order();
    String state = sCmd.next();
    myInputDigit->inputSetFloat(key, state);
}