#include "DeviceList.h"

void addThisDeviceToList() {
    jsonWriteStr_(devListHeapJson, "devicelist", "");  //метка для парсинга
    jsonWriteStr_(devListHeapJson, "ip", jsonReadStr(settingsFlashJson, "ip"));
    jsonWriteStr_(devListHeapJson, "id", jsonReadStr(settingsFlashJson, "id"));
    jsonWriteStr_(devListHeapJson, "name", jsonReadStr(settingsFlashJson, "name"));

    // для проверки
    // devListHeapJson = devListHeapJson + ",";
    // String test;
    // jsonWriteStr_(test, "ip", "192.168.88.88");
    // jsonWriteStr_(test, "id", "123456789");
    // jsonWriteStr_(test, "name", "test");
    // devListHeapJson = devListHeapJson + test;

    devListHeapJson = "[" + devListHeapJson + "]";
    Serial.println(devListHeapJson);
}