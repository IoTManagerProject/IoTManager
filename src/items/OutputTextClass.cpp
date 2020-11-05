#include "BufferExecute.h"
#include "items/OutputTextClass.h"
//===============================================Модуль вывода текста============================================
//output-text;id;anydata;Вывод;Сигнализация;order;st[Обнаружено.движение]
//===============================================================================================================
OutputTextClass myOutputText;
void textOut() {
    myOutputText.update();
    String key = myOutputText.gkey();
    sCmd.addCommand(key.c_str(), textOutSet);
    myOutputText.OutputModuleStateSetDefault();
    myOutputText.clear();
}

void textOutSet() {
    String key = sCmd.order();
    String state = sCmd.next();
    myOutputText.OutputModuleChange(key, state);
}