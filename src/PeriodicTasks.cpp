#include "PeriodicTasks.h"

void periodicTasksInit() {
    //задачи редкого выполнения
    ts.add(
        PTASK, 1000 * 30, [&](void*) {
            // heap
            String heap = prettyBytes(ESP.getFreeHeap());
            SerialPrint(F("i"), F("HEAP"), heap);
            printGlobalVarSize();
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

void printGlobalVarSize() {
    size_t settingsFlashJsonSize = settingsFlashJson.length();
    SerialPrint(F("i"), F("settingsFlashJson"), String(settingsFlashJsonSize));
    size_t errorsHeapJsonSize = errorsHeapJson.length();
    SerialPrint(F("i"), F("settingsFlashJson"), String(errorsHeapJsonSize));
    size_t paramsFlashJsonSize = paramsFlashJson.length();
    SerialPrint(F("i"), F("settingsFlashJson"), String(paramsFlashJsonSize));
    size_t paramsHeapJsonSize = paramsHeapJson.length();
    SerialPrint(F("i"), F("settingsFlashJson"), String(paramsHeapJsonSize));

    size_t halfBuffer = JSON_BUFFER_SIZE / 2;

    if (settingsFlashJsonSize > halfBuffer || errorsHeapJsonSize > halfBuffer || paramsFlashJsonSize > halfBuffer || paramsHeapJsonSize > halfBuffer) {
        SerialPrint(F("EE"), F("Json"), "Insufficient buffer size!!!");
        jsonWriteInt(errorsHeapJson, "jsbuf", 1);
    }
}