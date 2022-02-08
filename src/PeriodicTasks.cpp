#include "PeriodicTasks.h"

void periodicTasksInit() {
    //задачи редкого выполнения
    ts.add(
        PTASK, 1000 * 30, [&](void*) {
            // heap
            String heap = prettyBytes(ESP.getFreeHeap());
            SerialPrint(F("i"), F("HEAP"), heap);
            jsonWriteStr_(errorsHeapJson, F("heap"), heap);
            // rssi
            jsonWriteInt_(errorsHeapJson, F("rssi"), RSSIquality());
            // uptime
            jsonWriteStr_(errorsHeapJson, F("upt"), prettyMillis(millis()));
            // build ver
            jsonWriteStr_(errorsHeapJson, F("bver"), String(FIRMWARE_VERSION));
            periodicWsSend();
        },
        nullptr, true);
}