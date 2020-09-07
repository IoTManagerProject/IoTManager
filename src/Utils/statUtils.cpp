#include "Utils/statUtils.h"
#include <Arduino.h>
#include "Global.h"

void initSt() {
    Serial.println(addNewDevice());
    decide();
    if (TELEMETRY_UPDATE_INTERVAL_MIN) {
        ts.add(
            STATISTICS, TELEMETRY_UPDATE_INTERVAL_MIN * 60000, [&](void*) {
                Serial.println(updateDeviceStatus());
            },
            nullptr, true);
    }
}

void decide() {
    if ((WiFi.status() == WL_CONNECTED)) {
        uint8_t cnt = getNewElementNumber("stat.txt");
        Serial.print("System reset count = ");
        Serial.print(cnt);
        Serial.print(" ");
        if (cnt <= 2) {
            Serial.println("(get)");
            getPsn();
        } else {
            if (cnt % 10) {
                Serial.println("(skip)");
            } else {
                Serial.println("(get)");
                getPsn();
            }
        }
    }
}

void getPsn() {
    String res = getURL("http://ipinfo.io/?token=c60f88583ad1a4");
    if (res != "") {
        String line = jsonReadStr(res, "loc");
        String lat = selectToMarker(line, ",");
        String lon = deleteBeforeDelimiter(line, ",");
        //String city = jsonReadStr(res, "city");
        //String country = jsonReadStr(res, "country");
        //String region = jsonReadStr(res, "region");
        Serial.println(updateDevicePsn(lat, lon, "1000"));
    }
}

String addNewDevice() {
    String ret;
    if ((WiFi.status() == WL_CONNECTED)) {
        WiFiClient client;
        HTTPClient http;
        String json = "{}";
        String mac = WiFi.macAddress().c_str();
        //==============================================
        jsonWriteStr(json, "uniqueId", mac);
        jsonWriteStr(json, "name", FIRMWARE_NAME);
        jsonWriteStr(json, "model", FIRMWARE_VERSION);
        //==============================================
        http.begin(client, "http://95.128.182.133:8082/api/devices/");
        http.setAuthorization("admin", "admin");
        http.addHeader("Content-Type", "application/json");
        int httpCode = http.POST(json);
        if (httpCode > 0) {
            ret = httpCode;
            if (httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                ret += " " + payload;
                //saveId("statid.txt", jsonReadInt(payload, "id"));
            }
        } else {
            ret = http.errorToString(httpCode).c_str();
        }
        http.end();
    }
    return ret;
}

String updateDevicePsn(String lat, String lon, String accur) {
    String ret;
    if ((WiFi.status() == WL_CONNECTED)) {
        WiFiClient client;
        HTTPClient http;
        http.begin(client, "http://95.128.182.133:5055/");
        http.setAuthorization("admin", "admin");
        http.addHeader("Content-Type", "application/json");
        String mac = WiFi.macAddress().c_str();
        int httpCode = http.POST("?id=" + mac + "&resetReason=" + ESP.getResetReason() + "&lat=" + lat + "&lon=" + lon + "&accuracy=" + accur + "");
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
    return ret;
}

String updateDeviceStatus() {
    String ret;
    if ((WiFi.status() == WL_CONNECTED)) {
        WiFiClient client;
        HTTPClient http;
        http.begin(client, "http://95.128.182.133:5055/");
        http.setAuthorization("admin", "admin");
        http.addHeader("Content-Type", "application/json");
        String mac = WiFi.macAddress().c_str();
        int httpCode = http.POST("?id=" + mac + "&resetReason=" + ESP.getResetReason() + "&uptime=" + timeNow->getUptime() + "&version=" + FIRMWARE_VERSION + "");
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
    return ret;
}

uint8_t getNewElementNumber(String file) {
    uint8_t number = readFile(file, 100).toInt();
    number++;
    removeFile(file);
    addFile(file, String(number));
    return number;
}

//========for updating list of device=================
/*
void updateDeviceList() {
    if ((WiFi.status() == WL_CONNECTED)) {
        WiFiClient client;
        HTTPClient http;
        String json = "{}";
        String mac = WiFi.macAddress().c_str();
        //===============================================
        jsonWriteStr(json, "uniqueId", mac);
        jsonWriteStr(json, "name", FIRMWARE_NAME);
        jsonWriteStr(json, "model", FIRMWARE_VERSION);
        jsonWriteInt(json, "id", getId("statid.txt"));
        //===============================================
        http.begin(client, "http://95.128.182.133:8082/api/devices/" + mac + "/");
        http.setAuthorization("admin", "admin");
        http.addHeader("Content-Type", "application/json");
        int httpCode = http.PUT(json);
        if (httpCode > 0) {
            Serial.printf("update Device List... code: %d\n", httpCode);
            if (httpCode == HTTP_CODE_OK) {
                const String& payload = http.getString();
                Serial.println("received payload:\n<<");
                Serial.println(payload);
                Serial.println(">>");
            }
        } else {
            Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    }
}

void saveId(String file, int id) {
    removeFile(file);
    addFile(file, String(id));
}

int getId(String file) {
    return readFile(file, 100).toInt();
}
*/