#include "items/itemsGlobal.h"
#include "Class/OutputModule.h"
//===============================================Модуль вывода текста============================================
//output-text;id;anydata;Вывод;Сигнализация;order;st[Обнаружено.движение]
//===============================================================================================================
void textOut() {
    myText = new OutputModule();
    myText->update();
    String key = myText->gkey();
    sCmd.addCommand(key.c_str(), textOutSet);
    myText->OutputModuleStateSetDefault();
    myText->clear();
}

void textOutSet() {
    String key = sCmd.order();
    String state = sCmd.next();
    myText->OutputModuleChange(key, state);
}