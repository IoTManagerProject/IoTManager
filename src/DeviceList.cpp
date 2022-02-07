#include "DeviceList.h"

void addThisDeviceToList() {
    jsonWriteStr_(devListJson, "devicelist", "devicelist");
    jsonWriteStr_(devListJson, "ip", jsonReadStr(settingsFlashJson, "ip"));
    jsonWriteStr_(devListJson, "id", jsonReadStr(settingsFlashJson, "id"));
    jsonWriteStr_(devListJson, "name", jsonReadStr(settingsFlashJson, "name"));
    jsonWriteBool_(devListJson, "status", true);
    devListJson = "[" + devListJson + "]";
    Serial.println(devListJson);
}