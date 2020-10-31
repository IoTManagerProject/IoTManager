#include "Utils/StatUtils.h"

#include <Arduino.h>
#include <EEPROM.h>

#include "Global.h"
#include "ItemsList.h"

#ifdef ESP32
#include <rom/rtc.h>
#endif

String ESP_getResetReason(void);

void initSt() {
    addNewDevice();
    decide();
    if (TELEMETRY_UPDATE_INTERVAL_MIN) {
        ts.add(
            STATISTICS, TELEMETRY_UPDATE_INTERVAL_MIN * 60000, [&](void*) {
                static bool secondTime = false;
                if (secondTime) getNextNumber("totalhrs.txt");
                secondTime = true;
                updateDeviceStatus();
            },
            nullptr, true);
    }
}

void decide() {
    if ((WiFi.status() == WL_CONNECTED)) {
        uint8_t cnt = getNextNumber("stat.txt");
        SerialPrint("I", "Stat", "Total resets number: " + String(cnt));
        if (cnt <= 3) {
            Serial.println("(get)");
            getPsn();
        } else {
            if (cnt % 5) {
                Serial.println("(skip)");
            } else {
                Serial.println("(get)");
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
        updateDevicePsn(lat, lon, "0");
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
        jsonWriteInt(json, "model", FIRMWARE_VERSION);
        //==============================================
        http.begin(client,  serverIP + F(":8082/api/devices/"));
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
    SerialPrint("I", "Stat", "New device registaration: " + ret);
    return ret;
}

String updateDevicePsn(String lat, String lon, String accur) {
    String ret;
    if ((WiFi.status() == WL_CONNECTED)) {
        float latfl = lat.toFloat();
        float lonfl = lon.toFloat();
        randomSeed(micros());
        float latc = random(1, 9)/100;
        randomSeed(micros());
        float lonc = random(1, 9)/100;
        latfl = latfl + latc;
        lonfl = lonfl + lonc;
        WiFiClient client;
        HTTPClient http;
        http.begin(client,  serverIP + F(":5055/"));
        http.setAuthorization("admin", "admin");
        http.addHeader("Content-Type", "application/json");
        String mac = WiFi.macAddress().c_str();
        int httpCode = http.POST("?id=" + mac +
                                 "&resetReason=" + ESP_getResetReason() +
                                 "&lat=" + String(latfl) +
                                 "&lon=" + String(lonfl) +
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
    SerialPrint("I", "Stat", "Update device psn: " + ret);
    return ret;
}

String updateDeviceStatus() {
    String ret;
    if ((WiFi.status() == WL_CONNECTED)) {
        WiFiClient client;
        HTTPClient http;
        http.begin(client,  serverIP + F(":5055/"));
        http.setAuthorization("admin", "admin");
        http.addHeader("Content-Type", "application/json");
        String mac = WiFi.macAddress().c_str();
        int httpCode = http.POST("?id=" + mac +
                                 "&resetReason=" + ESP_getResetReason() +
                                 "&uptime=" + timeNow->getUptime() +
                                 "&uptimeTotal=" + getUptimeTotal() +
                                 "&version=" + FIRMWARE_VERSION +
                                 "&resetsTotal=" + String(getCurrentNumber("stat.txt")) + "");
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
    SerialPrint("I", "Stat", "Update device data: " + ret);
    return ret;
}

String getUptimeTotal() {
    uint8_t hrs = getCurrentNumber("totalhrs.txt");
    String hrsStr = prettySeconds(hrs * 60 * 60);
    SerialPrint("I", "Stat", "Total running time: " + hrsStr);
    return hrsStr;
}

uint8_t getNextNumber(String file) {
    uint8_t number = readFile(file, 100).toInt();
    number++;
    removeFile(file);
    addFile(file, String(number));
    return number;
}

uint8_t getCurrentNumber(String file) {
    uint8_t number = readFile(file, 100).toInt();
    return number;
}

#ifdef ESP8266
String ESP_getResetReason(void) {
    return ESP.getResetReason();
}
#else
String ESP32GetResetReason(uint32_t cpu_no) {
    // tools\sdk\include\esp32\rom\rtc.h
    switch (rtc_get_reset_reason((RESET_REASON)cpu_no)) {
        case POWERON_RESET:
            return F("Vbat power on reset");  // 1
        case SW_RESET:
            return F("Software reset digital core");  // 3
        case OWDT_RESET:
            return F("Legacy watch dog reset digital core");  // 4
        case DEEPSLEEP_RESET:
            return F("Deep Sleep reset digital core");  // 5
        case SDIO_RESET:
            return F("Reset by SLC module, reset digital core");  // 6
        case TG0WDT_SYS_RESET:
            return F("Timer Group0 Watch dog reset digital core");  // 7
        case TG1WDT_SYS_RESET:
            return F("Timer Group1 Watch dog reset digital core");  // 8
        case RTCWDT_SYS_RESET:
            return F("RTC Watch dog Reset digital core");  // 9
        case INTRUSION_RESET:
            return F("Instrusion tested to reset CPU");  // 10
        case TGWDT_CPU_RESET:
            return F("Time Group reset CPU");  // 11
        case SW_CPU_RESET:
            return F("Software reset CPU");  // 12
        case RTCWDT_CPU_RESET:
            return F("RTC Watch dog Reset CPU");  // 13
        case EXT_CPU_RESET:
            return F("or APP CPU, reseted by PRO CPU");  // 14
        case RTCWDT_BROWN_OUT_RESET:
            return F("Reset when the vdd voltage is not stable");  // 15
        case RTCWDT_RTC_RESET:
            return F("RTC Watch dog reset digital core and rtc module");  // 16
        default:
            return F("NO_MEAN");  // 0
    }
}

String ESP_getResetReason(void) {
    return ESP32GetResetReason(0);  // CPU 0
}
#endif
//String getUptimeTotal() {
//    static int hrs;
//    EEPROM.begin(512);
//    hrs = eeGetInt(0);
//    SerialPrint("I","Stat","Total running hrs: " + String(hrs));
//    String hrsStr = prettySeconds(hrs * 60 * 60);
//    SerialPrint("I","Stat","Total running hrs (f): " + hrsStr);
//    return hrsStr;
//}
//int plusOneHour() {
//    static int hrs;
//    EEPROM.begin(512);
//    hrs = eeGetInt(0);
//    hrs++;
//    eeWriteInt(0, hrs);
//    return hrs;
//}
//
//void eeWriteInt(int pos, int val) {
//    byte* p = (byte*)&val;
//    EEPROM.write(pos, *p);
//    EEPROM.write(pos + 1, *(p + 1));
//    EEPROM.write(pos + 2, *(p + 2));
//    EEPROM.write(pos + 3, *(p + 3));
//    EEPROM.commit();
//}
//
//int eeGetInt(int pos) {
//    int val;
//    byte* p = (byte*)&val;
//    *p = EEPROM.read(pos);
//    *(p + 1) = EEPROM.read(pos + 1);
//    *(p + 2) = EEPROM.read(pos + 2);
//    *(p + 3) = EEPROM.read(pos + 3);
//    if (val < 0) {
//        return 0;
//    } else {
//        return val;
//    }
//}
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
        http.begin(client, "http://") + serverIP + F(":8082/api/devices/" + mac + "/");
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