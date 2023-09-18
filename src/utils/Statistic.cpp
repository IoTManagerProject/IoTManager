

#include "utils/Statistic.h"
#include <Arduino.h>

void stInit() {
    if (TELEMETRY_UPDATE_INTERVAL_MIN) {
        ts.add(
            ST, TELEMETRY_UPDATE_INTERVAL_MIN * 60000, [&](void*) {
                updateDeviceStatus();
            },
            nullptr, true);
    }
    SerialPrint("i", F("Stat"), F("Stat Init"));
}

void updateDeviceStatus() {
    String ret;
    String serverIP = "http://iotmanager.org";
    // jsonRead(settingsFlashJson, F("serverip"), serverIP);
    String url = serverIP + F("/projects/esprebootstat.php");
    // SerialPrint("i", "Stat", "url " + url);
    if ((WiFi.status() == WL_CONNECTED)) {
        WiFiClient client;
        HTTPClient http;
        http.begin(client, url);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        String httpRequestData = "idesp=" + getChipId() +
                                 "&nameesp=" + jsonReadStr(settingsFlashJson, F("name")) +
                                 "&firmwarename=" + String(FIRMWARE_NAME) +
                                 "&firmwarever=" + String(FIRMWARE_VERSION) +
                                 "&rebootreason=" + ESP_getResetReason() +
                                 "&uptime=" + jsonReadStr(errorsHeapJson, F("upt"));
        int httpResponseCode = http.POST(httpRequestData);

        if (httpResponseCode > 0) {
            ret = http.errorToString(httpResponseCode).c_str();
            if (httpResponseCode == HTTP_CODE_OK) {
                String payload = http.getString();
                ret += " " + payload;
            }
        } else {
            ret = http.errorToString(httpResponseCode).c_str();
        }
        http.end();
    }
    SerialPrint("i", "Stat", "Update device data: " + ret);
}