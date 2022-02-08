#include "PeriodicTasks.h"

void periodicTasksInit() {
    //задачи редкого выполнения
    ts.add(
        PTASK, 1000 * 30, [&](void*) {
            SerialPrint(F("i"), F("HEAP"), prettyBytes(ESP.getFreeHeap()));
            periodicWsSend();
        },
        nullptr, true);
}