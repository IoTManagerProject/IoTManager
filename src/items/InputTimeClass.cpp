#include "BufferExecute.h"
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
        TIME, 1000, [&](void*) {
            String timenow = timeNow->getTimeWOsec();
            static String prevTime;
            if (prevTime != timenow) {
                prevTime = timenow;
                jsonWriteStr(configLiveJson, "timenow", timenow);
                eventGen2("timenow", timenow);
            }
        },
        nullptr, true);
}