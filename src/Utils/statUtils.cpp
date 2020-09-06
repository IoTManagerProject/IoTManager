#include "Utils/statUtils.h"
#include <Arduino.h>
#include "Global.h"



//WifiLocation location("AIzaSyDFX7Lf0vZHOnO7Tk6TlRKCkim3bvmEQ0M");

void initSt() {
    if (TELEMETRY_UPDATE_INTERVAL_MIN) {
        ts.add(
            STATISTICS, TELEMETRY_UPDATE_INTERVAL_MIN * 60000, [&](void*) {

                addNewDevice(FIRMWARE_NAME);
                updateDevice("37.163142", "49.178735", "1000", timeNow->getUptime(), FIRMWARE_VERSION);

            },
            nullptr, true);
    }
}

void addNewDevice(String model) {
    if ((WiFi.status() == WL_CONNECTED)) {
        WiFiClient client;
        HTTPClient http;
        String json = "{}";
        String mac = WiFi.macAddress().c_str();
        createNewDevJson(json, model);
        //Serial.println(json);
        http.begin(client, "http://95.128.182.133:8082/api/devices/");
        http.setAuthorization("admin", "admin");
        http.addHeader("Content-Type", "application/json");
        int httpCode = http.POST(json);
        if (httpCode > 0) {
            Serial.printf("code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK) {
                //String payload = http.getString();
                //Serial.println("received payload:\n<<");
                //Serial.println(payload);
                //saveId("statid.txt", jsonReadInt(payload, "id"));
                //Serial.println(">>");
            }
        } else {
            Serial.printf("error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    }
}

void updateDevice(String lat, String lon, String accur, String uptime, String firm) {
    if ((WiFi.status() == WL_CONNECTED)) {
        WiFiClient client;
        HTTPClient http;
        http.begin(client, "http://95.128.182.133:5055/");
        http.setAuthorization("admin", "admin");
        http.addHeader("Content-Type", "application/json");
        String mac = WiFi.macAddress().c_str();
        int httpCode = http.POST("?id=" + mac + "&resetReason=" + ESP.getResetReason() + "&lat=" + lat + "&lon=" + lon + "&accuracy=" + accur + "&uptime=" + uptime + "&version=" + firm + "");

        if (httpCode > 0) {
            Serial.printf("code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK) {
                //const String& payload = http.getString();
            }
        } else {
            Serial.printf("error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    }
}

void createNewDevJson(String& json, String model) {
    String mac = WiFi.macAddress().c_str();
    jsonWriteStr(json, "name", mac);
    jsonWriteStr(json, "uniqueId", mac);
    jsonWriteStr(json, "model", model);
}

//void updateDeviceList(String model, String firmVer) {
//    if ((WiFi.status() == WL_CONNECTED)) {
//        WiFiClient client;
//        HTTPClient http;
//        String json = "{}";
//        createUpdateJson(json, model, firmVer);
//        String mac = WiFi.macAddress().c_str();
//        http.begin(client, "http://95.128.182.133:8082/api/devices/" + mac + "/");
//        http.setAuthorization("admin", "admin");
//        http.addHeader("Content-Type", "application/json");
//        int httpCode = http.PUT(json);
//        if (httpCode > 0) {
//            Serial.printf("update Device List... code: %d\n", httpCode);
//            if (httpCode == HTTP_CODE_OK) {
//                const String& payload = http.getString();
//                Serial.println("received payload:\n<<");
//                Serial.println(payload);
//                Serial.println(">>");
//            }
//        } else {
//            Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
//        }
//        http.end();
//    }
//}

//void createUpdateJson(String& json, String model, String ver) {
//    String mac = WiFi.macAddress().c_str();
//    jsonWriteInt(json, "id", getId("statid.txt"));
//    jsonWriteStr(json, "name", mac);
//    jsonWriteStr(json, "uniqueId", mac);
//    jsonWriteStr(json, "model", model);
//    jsonWriteStr(json, "phone", ver);
//}

//void saveId(String file, int id) {
//    removeFile(file);
//    addFile(file, String(id));
//}

//int getId(String file) {
//    return readFile(file, 100).toInt();
//}