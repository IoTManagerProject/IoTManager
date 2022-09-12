

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
    String serverIP;
    jsonRead(settingsFlashJson, F("serverip"), serverIP);
    String url = serverIP + F("/projects/esprebootstat.php");
    // SerialPrint("i", "Stat", "url " + url);
    if ((WiFi.status() == WL_CONNECTED)) {
        WiFiClient client;
        HTTPClient http;
        http.begin(client, url);
        // http.setAuthorization("admin", "admin");
        http.addHeader("Content-Type", "application/json");
        // String mac = WiFi.macAddress().c_str();
        String json = "";
        jsonWriteStr_(json, "idesp", getChipId());
        jsonWriteStr_(json, "nameesp", jsonReadStr(settingsFlashJson, F("name")));
        jsonWriteStr_(json, "firmwarename", String(FIRMWARE_NAME));
        jsonWriteStr_(json, "firmwarever", String(FIRMWARE_VERSION));
        jsonWriteStr_(json, "rebootreason", ESP_getResetReason());
        int httpCode = http.POST(json);
        // int httpCode = http.POST("?idesp=" + getChipId() +
        //                          "&nameesp=" + jsonReadStr(settingsFlashJson, F("name")) +
        //                          "&firmwarename=" + String(FIRMWARE_NAME) +
        //                          "&firmwarever=" + String(FIRMWARE_VERSION) +
        //                          "&rebootreason=" + ESP_getResetReason() + "");
        if (httpCode > 0) {
            ret = httpCode;
            if (httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                ret += " " + payload;
            }
        } else {
            ret = http.errorToString(httpCode).c_str();
        }
        http.end();
    }
    SerialPrint("i", "Stat", "Update device data: " + ret);
}