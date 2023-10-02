#include "PeriodicTasks.h"

void periodicTasksInit() {
    //задачи редкого выполнения
    ts.add(
        PTASK, 1000 * 60, [&](void*) {
            // fs
            getFSInfo();
            // heap
            String heap = prettyBytes(ESP.getFreeHeap());
            SerialPrint(F("i"), F("HEAP"), heap);
            SerialPrint(F("i"), F("IoTItems"), (String)IoTItems.size());
            printGlobalVarSize();
            jsonWriteStr_(errorsHeapJson, F("heap"), heap);
            // rssi
            jsonWriteInt_(errorsHeapJson, F("rssi"), RSSIquality());
            // uptime
            jsonWriteStr_(errorsHeapJson, F("upt"), prettyMillis(millis()));
            jsonWriteStr_(errorsHeapJson, F("uptm"), prettyMillis(mqttUptime));
            jsonWriteStr_(errorsHeapJson, F("uptw"), prettyMillis(wifiUptime));
            // flash
            jsonWriteInt_(errorsHeapJson, F("fl"), flashWriteNumber);
            // build ver
            jsonWriteStr_(errorsHeapJson, F("bver"), String(FIRMWARE_VERSION));
            jsonWriteStr_(errorsHeapJson, F("bn"), String(FIRMWARE_NAME));
            jsonWriteStr_(errorsHeapJson, F("wver"), getWebVersion());
            // reset reason
            jsonWriteStr_(errorsHeapJson, F("rst"), ESP_getResetReason());
            periodicWsSend();
        },
        nullptr, true);
    SerialPrint("i", "Task", "Periodic tasks init");
}

void printGlobalVarSize() {
    size_t settingsFlashJsonSize = settingsFlashJson.length();
    // SerialPrint(F("i"), F("settingsFlashJson"), String(settingsFlashJsonSize));
    size_t valuesFlashJsonSize = valuesFlashJson.length();
    size_t errorsHeapJsonSize = errorsHeapJson.length();
    // SerialPrint(F("i"), F("errorsHeapJson"), String(errorsHeapJsonSize));
    size_t devListHeapJsonSize = devListHeapJson.length();
    // SerialPrint(F("i"), F("devListHeapJson"), String(devListHeapJsonSize));

    SerialPrint(F("i"), F("Var summ sz"), String(settingsFlashJsonSize + valuesFlashJsonSize + errorsHeapJsonSize + devListHeapJsonSize));

    size_t halfBuffer = JSON_BUFFER_SIZE / 2;

    if (settingsFlashJsonSize > halfBuffer ||
        valuesFlashJsonSize > halfBuffer ||
        errorsHeapJsonSize > halfBuffer ||
        devListHeapJsonSize > halfBuffer) {
        SerialPrint(F("EE"), F("Json"), F("Insufficient buffer size!!!"));
        jsonWriteInt(errorsHeapJson, "jse1", 1);
    }
}

#ifdef ESP8266
String ESP_getResetReason(void) {
    return ESP.getResetReason();
}
#endif
#ifdef esp32s2_4mb
String ESP_getResetReason(void) {
    return ESP32GetResetReason(0);  // CPU 0
}
String ESP32GetResetReason(uint32_t cpu_no) {
    // tools\sdk\include\esp32\rom\rtc.h
    switch (rtc_get_reset_reason((RESET_REASON)cpu_no)) {
        case POWERON_RESET:
            return F("Vbat power on reset");  // 1
        case RTC_SW_SYS_RESET:
            return F("Software reset digital core");  // 3
//      case OWDT_RESET:
//          return F("Legacy Watchdog reset digital core");  // 4
        case DEEPSLEEP_RESET:
            return F("Deep Sleep reset digital core");  // 5
//      case SDIO_RESET:
//          return F("Reset by SLC module, reset digital core");  // 6
        case TG0WDT_SYS_RESET:
            return F("Timer Group0 Watchdog reset digital core");  // 7
        case TG1WDT_SYS_RESET:
            return F("Timer Group1 Watchdog reset digital core");  // 8
        case RTCWDT_SYS_RESET:
            return F("RTC Watchdog Reset digital core");  // 9
        case INTRUSION_RESET:
            return F("Instrusion tested to reset CPU");  // 10
        case TG0WDT_CPU_RESET:
            return F("Time Group reset CPU");  // 11
        case RTC_SW_CPU_RESET:
            return F("Software reset CPU");  // 12
        case RTCWDT_CPU_RESET:
            return F("RTC Watchdog Reset CPU");  // 13
//      case EXT_CPU_RESET:
//          return F("or APP CPU, reseted by PRO CPU");  // 14
        case RTCWDT_BROWN_OUT_RESET:
            return F("Reset when the vdd voltage is not stable");  // 15
        case RTCWDT_RTC_RESET:
            return F("RTC Watchdog reset digital core and rtc module");  // 16
        default:
            return F("NO_MEAN");  // 0
    }
}
#endif
#if defined(esp32_4mb) || defined(esp32_16mb) 
String ESP_getResetReason(void) {
    return ESP32GetResetReason(0);  // CPU 0
}
String ESP32GetResetReason(uint32_t cpu_no) {
    // tools\sdk\include\esp32\rom\rtc.h
    switch (rtc_get_reset_reason((RESET_REASON)cpu_no)) {
        case POWERON_RESET:
            return F("Vbat power on reset");  // 1
        case SW_RESET:
            return F("Software reset digital core");  // 3
        case OWDT_RESET:
            return F("Legacy Watchdog reset digital core");  // 4
        case DEEPSLEEP_RESET:
            return F("Deep Sleep reset digital core");  // 5
        case SDIO_RESET:
            return F("Reset by SLC module, reset digital core");  // 6
        case TG0WDT_SYS_RESET:
            return F("Timer Group0 Watchdog reset digital core");  // 7
        case TG1WDT_SYS_RESET:
            return F("Timer Group1 Watchdog reset digital core");  // 8
        case RTCWDT_SYS_RESET:
            return F("RTC Watchdog Reset digital core");  // 9
        case INTRUSION_RESET:
            return F("Instrusion tested to reset CPU");  // 10
        case TGWDT_CPU_RESET:
            return F("Time Group reset CPU");  // 11
        case SW_CPU_RESET:
            return F("Software reset CPU");  // 12
        case RTCWDT_CPU_RESET:
            return F("RTC Watchdog Reset CPU");  // 13
        case EXT_CPU_RESET:
            return F("or APP CPU, reseted by PRO CPU");  // 14
        case RTCWDT_BROWN_OUT_RESET:
            return F("Reset when the vdd voltage is not stable");  // 15
        case RTCWDT_RTC_RESET:
            return F("RTC Watchdog reset digital core and rtc module");  // 16
        default:
            return F("NO_MEAN");  // 0
    }
}
#endif