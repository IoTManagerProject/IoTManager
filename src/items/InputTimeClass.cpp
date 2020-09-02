#include "items/itemsGlobal.h"
#include "items/InputClass.h"
//==========================================Модуль ввода времени============================================
//input-time time1 inputTime Ввод Введите.время 4 st[10-00-00]
//==========================================================================================================
InputClass myInputTime;
void inputTime() {
    myInputTime.update();
    String key = myInputTime.gkey();
    sCmd.addCommand(key.c_str(), inputTimeSet);
    myInputTime.inputSetDefaultStr();
    myInputTime.clear();
}

void inputTimeSet() {
    String key = sCmd.order();
    String state = sCmd.next();
    myInputTime.inputSetStr(key, state);
}

void handle_time_init() {
    ts.add(
        TIME, 1000, [&](void *) {
            jsonWriteStr(configLiveJson, "time", timeNow->getTime());
            jsonWriteStr(configLiveJson, "timenow", timeNow->getTimeJson());
            //eventGen("timenow", "");
        },
        nullptr, true);
}