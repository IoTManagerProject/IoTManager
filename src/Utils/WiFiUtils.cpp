#include "Utils/WiFiUtils.h"



void startSTAMode() {
    setLedStatus(LED_SLOW);
    SerialPrint("I","module","STA Mode");

    String ssid = jsonReadStr(configSetupJson, "routerssid");
    String passwd = jsonReadStr(configSetupJson, "routerpass");

    WiFi.mode(WIFI_STA);
    if (ssid == "" && passwd == "") {
        WiFi.begin();
    } else {
        if (WiFi.begin(ssid.c_str(), passwd.c_str()) == WL_CONNECT_FAILED) {
            SerialPrint("[E]","module","failed on start");
        }
    }

    bool keepConnecting = true;
    uint8_t tries = 20;
    sint8_t connRes;
    do {
#ifdef ESP8266
        connRes = WiFi.waitForConnectResult(1000);
#else
        byte connRes = WiFi.waitForConnectResult();
#endif
        switch (connRes) {
            case WL_NO_SSID_AVAIL: {
                SerialPrint("[E]","module","no network");
                keepConnecting = false;
            } break;
            case WL_CONNECTED: {
                String hostIpStr = WiFi.localIP().toString();
                SerialPrint("I","module","http://" + hostIpStr);
                jsonWriteStr(configSetupJson, "ip", hostIpStr);
                keepConnecting = false;
            } break;
            case WL_CONNECT_FAILED: {
                SerialPrint("[E]","module","check credentials");
                jsonWriteInt(configOptionJson, "pass_status", 1);
                keepConnecting = false;
            } break;
            default:
                break;
        }
    } while (keepConnecting && tries--);

    if (isNetworkActive()) {
        MqttClient::init();
        setLedStatus(LED_OFF);
    } else {
        SerialPrint("[E]","module","failed: " + String(connRes, DEC));
        startAPMode();
    };
}

bool startAPMode() {
    setLedStatus(LED_ON);
    SerialPrint("I","module","AP Mode");

    String ssid = jsonReadStr(configSetupJson, "apssid");
    String passwd = jsonReadStr(configSetupJson, "appass");

    WiFi.mode(WIFI_AP);

    WiFi.softAP(ssid.c_str(), passwd.c_str());
    String hostIpStr = WiFi.softAPIP().toString();
    SerialPrint("I","module","Host IP: " + hostIpStr);
    jsonWriteStr(configSetupJson, "ip", hostIpStr);

    ts.add(
        WIFI_SCAN, 10 * 1000,
        [&](void*) {
            String sta_ssid = jsonReadStr(configSetupJson, "routerssid");
            SerialPrint("I","module","scanning for " + sta_ssid);
            if (scanWiFi(sta_ssid)) {
                ts.remove(WIFI_SCAN);
                startSTAMode();
            }
        },
        nullptr, true);

    return true;
}

boolean scanWiFi(String ssid) {
    bool res = false;
    int8_t n = WiFi.scanComplete();
    SerialPrint("I","module","scan result: " + String(n, DEC));
    if (n == -2) {
        // не было запущено, запускаем
        SerialPrint("I","module","start scanning");
        // async, show_hidden
        WiFi.scanNetworks(true, false);
    } else if (n == -1) {
        // все еще выполняется
        SerialPrint("I","module","scanning in progress");
    } else if (n == 0) {
        // не найдена ни одна сеть
        SerialPrint("I","module","no networks found");
        WiFi.scanNetworks(true, false);
    } else if (n > 0) {
        for (int8_t i = 0; i < n; i++) {
            if (WiFi.SSID(i) == ssid) {
                res = true;
            }
            SerialPrint("I","module",(res ? "*" : "") + String(i, DEC) + ") " + WiFi.SSID(i));
        }
    }
    return res;
}

boolean isNetworkActive() {
    return WiFi.status() == WL_CONNECTED;
}
