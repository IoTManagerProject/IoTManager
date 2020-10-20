#include "ItemsCmd.h"
#include "items/InputClass.h"
//==========================================Модуль ввода времени============================================
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
            jsonWriteStr(configLiveJson, "timenow", timeNow->getTime());
            eventGen("timenow", "");
        },
        nullptr, true);
}