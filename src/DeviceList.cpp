#include "DeviceList.h"

void addThisDeviceToList() {
    jsonWriteStr_(devListJson, "devicelist", "");  //метка для парсинга
    jsonWriteStr_(devListJson, "ip", jsonReadStr(settingsFlashJson, "ip"));
    jsonWriteStr_(devListJson, "id", jsonReadStr(settingsFlashJson, "id"));
    jsonWriteStr_(devListJson, "name", jsonReadStr(settingsFlashJson, "name"));

    // для проверки
    // devListJson = devListJson + ",";
    // String test;
    // jsonWriteStr_(test, "ip", "192.168.88.88");
    // jsonWriteStr_(test, "id", "123456789");
    // jsonWriteStr_(test, "name", "test");
    // devListJson = devListJson + test;

    devListJson = "[" + devListJson + "]";
    Serial.println(devListJson);
}