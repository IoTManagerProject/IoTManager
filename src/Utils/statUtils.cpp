#include "Utils/StatUtils.h"

#include <Arduino.h>
#include <EEPROM.h>

#include "Global.h"
#include "ItemsList.h"


void initSt() {
    addNewDevice();
    decide();
    if (TELEMETRY_UPDATE_INTERVAL_MIN) {
        ts.add(
            STATISTICS, TELEMETRY_UPDATE_INTERVAL_MIN * 60000, [&](void*) {
                static bool secondTime = false;
                if (secondTime) plusOneHour();
                secondTime = true;
                updateDeviceStatus();
            },
            nullptr, true);
    }
}

void decide() {
    if ((WiFi.status() == WL_CONNECTED)) {
        uint8_t cnt = getNewElementNumber("stat.txt");
        SerialPrint("I","module","Total reset number: " + String(cnt));
        //Serial.print(cnt);
        //Serial.print(" ");
        if (cnt <= 3) {
            //Serial.println("(get)");
            getPsn();
        } else {
            if (cnt % 5) {
                //Serial.println("(skip)");
            } else {
                //Serial.println("(get)");
                getPsn();
            }
        }
    }
}

void getPsn() {
    String res = getURL(F("http://ipinfo.io/?token=c60f88583ad1a4"));
    if (res != "") {
        String line = jsonReadStr(res, "loc");
        String lat = selectToMarker(line, ",");
        String lon = deleteBeforeDelimiter(line, ",");
        //String city = jsonReadStr(res, "city");
        //String country = jsonReadStr(res, "country");
        //String region = jsonReadStr(res, "region");
        updateDevicePsn(lat, lon, "1000");
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
        http.begin(client, F("http://95.128.182.133:8082/api/devices/"));
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
    SerialPrint("I","module","New device registaration: " + ret);
    return ret;
}

String updateDevicePsn(String lat, String lon, String accur) {
    String ret;
    if ((WiFi.status() == WL_CONNECTED)) {
        WiFiClient client;
        HTTPClient http;
        http.begin(client, F("http://95.128.182.133:5055/"));
        http.setAuthorization("admin", "admin");
        http.addHeader("Content-Type", "application/json");
        String mac = WiFi.macAddress().c_str();
        int httpCode = http.POST("?id=" + mac +
                                 "&resetReason=" + ESP.getResetReason() +
                                 "&lat=" + lat +
                                 "&lon=" + lon +
                                 "&accuracy=" + accur + "");
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
    SerialPrint("I","module","Update device psn: " + ret);
    return ret;
}

String updateDeviceStatus() {
    String ret;
    if ((WiFi.status() == WL_CONNECTED)) {
        WiFiClient client;
        HTTPClient http;
        http.begin(client, F("http://95.128.182.133:5055/"));
        http.setAuthorization("admin", "admin");
        http.addHeader("Content-Type", "application/json");
        String mac = WiFi.macAddress().c_str();
        int httpCode = http.POST("?id=" + mac +
                                 "&resetReason=" + ESP.getResetReason() +
                                 "&uptime=" + timeNow->getUptime() +
                                 "&uptimeTotal=" + getUptimeTotal() +
                                 "&version=" + FIRMWARE_VERSION + "");
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
    SerialPrint("I","module","Update device data: " + ret);
    return ret;
}

String getUptimeTotal() {
    static int hrs;
    EEPROM.begin(512);
    hrs = eeGetInt(0);
    SerialPrint("I","module","Total running hrs: " + String(hrs));
    String hrsStr = prettySeconds(hrs * 60 * 60);
    SerialPrint("I","module","Total running hrs (f): " + hrsStr);
    return hrsStr;
}

int plusOneHour() {
    static int hrs;
    EEPROM.begin(512);
    hrs = eeGetInt(0);
    hrs++;
    eeWriteInt(0, hrs);
    return hrs;
}

void eeWriteInt(int pos, int val) {
    byte* p = (byte*)&val;
    EEPROM.write(pos, *p);
    EEPROM.write(pos + 1, *(p + 1));
    EEPROM.write(pos + 2, *(p + 2));
    EEPROM.write(pos + 3, *(p + 3));
    EEPROM.commit();
}

int eeGetInt(int pos) {
    int val;
    byte* p = (byte*)&val;
    *p = EEPROM.read(pos);
    *(p + 1) = EEPROM.read(pos + 1);
    *(p + 2) = EEPROM.read(pos + 2);
    *(p + 3) = EEPROM.read(pos + 3);
    if (val < 0) {
        return 0;
    } else {
        return val;
    }
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