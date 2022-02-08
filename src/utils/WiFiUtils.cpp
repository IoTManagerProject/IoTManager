#include "Utils/WiFiUtils.h"

void routerConnect() {
    WiFi.setAutoConnect(false);
    WiFi.persistent(false);

    WiFi.mode(WIFI_STA);
    byte tries = 40;

    String _ssid = jsonReadStr(settingsFlashJson, "routerssid");
    String _password = jsonReadStr(settingsFlashJson, "routerpass");

    if (_ssid == "" && _password == "") {
        WiFi.begin();
    } else {
        WiFi.begin(_ssid.c_str(), _password.c_str());
#ifdef ESP32
        WiFi.setTxPower(WIFI_POWER_19_5dBm);
#else
        WiFi.setOutputPower(20.5);
#endif
        SerialPrint("i", "WIFI", "ssid: " + _ssid);
        SerialPrint("i", "WIFI", "pass: " + _password);
    }

    while (--tries && WiFi.status() != WL_CONNECTED) {
        if (WiFi.status() == WL_CONNECT_FAILED) {
            SerialPrint("E", "WIFI", "password is not correct");
            tries = 1;
            jsonWriteInt(errorsHeapJson, "passer", 1);
        }
        Serial.print(".");
        delay(1000);
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("");
        startAPMode();
    } else {
        Serial.println("");
        SerialPrint("i", "WIFI", "http://" + WiFi.localIP().toString());
        jsonWriteStr(settingsFlashJson, "ip", WiFi.localIP().toString());

        // mqttInit();
    }
    SerialPrint("i", F("WIFI"), F("Network Init"));
}

bool startAPMode() {
    SerialPrint("i", "WIFI", "AP Mode");

    WiFi.disconnect();
    WiFi.mode(WIFI_AP);

    String _ssidAP = jsonReadStr(settingsFlashJson, "apssid");
    String _passwordAP = jsonReadStr(settingsFlashJson, "appass");

    WiFi.softAP(_ssidAP.c_str(), _passwordAP.c_str());
    IPAddress myIP = WiFi.softAPIP();

    SerialPrint("i", "WIFI", "AP IP: " + myIP.toString());
    jsonWriteStr(settingsFlashJson, "ip", myIP.toString());

    if (jsonReadInt(errorsHeapJson, "passer") != 1) {
        ts.add(
            WIFI_SCAN, 10 * 1000, [&](void*) {
                String sta_ssid = jsonReadStr(settingsFlashJson, "routerssid");

                SerialPrint("i", "WIFI", "scanning for " + sta_ssid);

                if (RouterFind(sta_ssid)) {
                    ts.remove(WIFI_SCAN);
                    WiFi.scanDelete();
                    routerConnect();
                }
            },
            nullptr, true);
    }
    return true;
}

boolean RouterFind(String ssid) {
    bool res = false;
    int n = WiFi.scanComplete();
    SerialPrint("i", "WIFI", "scan result: " + String(n, DEC));

    if (n == -2) {  //Сканирование не было запущено, запускаем
        SerialPrint("i", "WIFI", "start scanning");
        WiFi.scanNetworks(true, false);  // async, show_hidden
    }

    else if (n == -1) {  //Сканирование все еще выполняется
        SerialPrint("i", "WIFI", "scanning in progress");
    }

    else if (n == 0) {  //ни одна сеть не найдена
        SerialPrint("i", "WIFI", "no networks found");
        WiFi.scanNetworks(true, false);
    }

    else if (n > 0) {
        for (int8_t i = 0; i < n; i++) {
            if (WiFi.SSID(i) == ssid) {
                res = true;
            }
            // SerialPrint("i", "WIFI", (res ? "*" : "") + String(i, DEC) + ") " + WiFi.SSID(i));
            jsonWriteStr_(ssidListHeapJson, String(i), WiFi.SSID(i));

            // String(WiFi.RSSI(i)
        }
    }
    SerialPrint("i", "WIFI", ssidListHeapJson);
    WiFi.scanDelete();
    return res;
}

boolean isNetworkActive() {
    return WiFi.status() == WL_CONNECTED;
}

uint8_t RSSIquality() {
    uint8_t res = 0;
    if (WiFi.status() == WL_CONNECTED) {
        int rssi = WiFi.RSSI();
        if (rssi >= -50) {
            res = 6;  //"Excellent";
        } else if (rssi < -50 && rssi >= -60) {
            res = 5;  //"Very good";
        } else if (rssi < -60 && rssi >= -70) {
            res = 4;  //"Good";
        } else if (rssi < -70 && rssi >= -80) {
            res = 3;  //"Low";
        } else if (rssi < -80 && rssi > -100) {
            res = 2;  //"Very low";
        } else if (rssi <= -100) {
            res = 1;  //"No signal";
        }
    }
    return res;
}

// void wifiSignalInit() {
//     ts.add(
//         SYGNAL, 1000 * 60, [&](void*) {
//             switch (RSSIquality()) {
//                 case 0:
//                     jsonWriteStr(settingsFlashJson, F("signal"), F("не подключено к роутеру"));
//                     break;
//                 case 1:
//                     jsonWriteStr(settingsFlashJson, F("signal"), F("нет сигнала"));
//                     break;
//                 case 2:
//                     jsonWriteStr(settingsFlashJson, F("signal"), F("очень низкий"));
//                     break;
//                 case 3:
//                     jsonWriteStr(settingsFlashJson, F("signal"), F("низкий"));
//                     break;
//                 case 4:
//                     jsonWriteStr(settingsFlashJson, F("signal"), F("хороший"));
//                     break;
//                 case 5:
//                     jsonWriteStr(settingsFlashJson, F("signal"), F("очень хороший"));
//                     break;
//                 case 6:
//                     jsonWriteStr(settingsFlashJson, F("signal"), F("отличный"));
//                     break;
//             }
//         },
//         nullptr, true);
// }
