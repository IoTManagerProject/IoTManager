#include "Utils/WiFiUtils.h"
#include "items/sysUptime.h"
#include "FileSystem.h"

void routerConnect() {
    WiFi.setAutoConnect(false);
    WiFi.persistent(false);

    setLedStatus(LED_SLOW);
    WiFi.mode(WIFI_STA);
    byte tries = 20;

    String _ssid = jsonReadStr(configSetupJson, "routerssid");
    String _password = jsonReadStr(configSetupJson, "routerpass");

    if (_ssid == "" && _password == "") {
        WiFi.begin();
    } else {
        WiFi.begin(_ssid.c_str(), _password.c_str());
#ifdef ESP32
        WiFi.setTxPower(WIFI_POWER_19_5dBm);
#else
        WiFi.setOutputPower(20.5);
#endif
        SerialPrint("I", "WIFI", "ssid: " + _ssid);
    }

    while (--tries && WiFi.status() != WL_CONNECTED) {
        if (WiFi.status() == WL_CONNECT_FAILED) {
            SerialPrint("E", "WIFI", "password is not correct");
            tries = 1;
            jsonWriteInt(configOptionJson, "pass_status", 1);
        }
        Serial.print(".");
        delay(1000);
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("");
        startAPMode();
    } else {
        Serial.println("");
        SerialPrint("I", "WIFI", "http://" + WiFi.localIP().toString());
        jsonWriteStr(configSetupJson, "ip", WiFi.localIP().toString());
        setLedStatus(LED_OFF);
        mqttInit();
    }
    SerialPrint("I", F("WIFI"), F("Network Init"));
}

bool startAPMode() {
    setLedStatus(LED_ON);
    SerialPrint("I", "WIFI", "AP Mode");

    WiFi.disconnect();
    WiFi.mode(WIFI_AP);

    String _ssidAP = jsonReadStr(configSetupJson, "apssid");
    String _passwordAP = jsonReadStr(configSetupJson, "appass");

    WiFi.softAP(_ssidAP.c_str(), _passwordAP.c_str());
    IPAddress myIP = WiFi.softAPIP();

    SerialPrint("I", "WIFI", "AP IP: " + myIP.toString());
    jsonWriteStr(configSetupJson, "ip", myIP.toString());

    //if (jsonReadInt(configOptionJson, "pass_status") != 1) {
    ts.add(
        WIFI_SCAN, 10 * 1000, [&](void*) {
            String sta_ssid = jsonReadStr(configSetupJson, "routerssid");

            SerialPrint("I", "WIFI", "scanning for " + sta_ssid);

            if (RouterFind(sta_ssid)) {
                ts.remove(WIFI_SCAN);
                WiFi.scanDelete();
                routerConnect();
            }
        },
        nullptr, true);
    //}
    return true;
}

boolean RouterFind(String ssid) {
    bool res = false;
    int n = WiFi.scanComplete();
    SerialPrint("I", "WIFI", "scan result: " + String(n, DEC));

    if (n == -2) {  //Сканирование не было запущено, запускаем
        SerialPrint("I", "WIFI", "start scanning");
        WiFi.scanNetworks(true, false);  //async, show_hidden
    }

    else if (n == -1) {  //Сканирование все еще выполняется
        SerialPrint("I", "WIFI", "scanning in progress");
    }

    else if (n == 0) {  //ни одна сеть не найдена
        SerialPrint("I", "WIFI", "no networks found");
        WiFi.scanNetworks(true, false);
    }

    else if (n > 0) {
        for (int8_t i = 0; i < n; i++) {
            if (WiFi.SSID(i) == ssid) {
                res = true;
            }
            SerialPrint("I", "WIFI", (res ? "*" : "") + String(i, DEC) + ") " + WiFi.SSID(i));
        }
    }
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

void wifiSignalInit() {
    ts.add(
        SYGNAL, 1000 * 60, [&](void*) {
            //uptimeReading();
            SerialPrint("I", "System", printMemoryStatus());
#ifdef ESP8266
            getFSInfo();
#endif
            switch (RSSIquality()) {
                case 0:
                    jsonWriteStr(configSetupJson, F("signal"), F("Уровень WiFi сигнала: <font color='red'>не подключено к роутеру</font>"));
                    break;
                case 1:
                    jsonWriteStr(configSetupJson, F("signal"), F("Уровень WiFi сигнала: <font color='red'>нет сигнала</font>"));
                    break;
                case 2:
                    jsonWriteStr(configSetupJson, F("signal"), F("Уровень WiFi сигнала: <font color='red'>очень низкий</font>"));
                    break;
                case 3:
                    jsonWriteStr(configSetupJson, F("signal"), F("Уровень WiFi сигнала: <font color='orange'>низкий</font>"));
                    break;
                case 4:
                    jsonWriteStr(configSetupJson, F("signal"), F("Уровень WiFi сигнала: <font color='green'>хороший</font>"));
                    break;
                case 5:
                    jsonWriteStr(configSetupJson, F("signal"), F("Уровень WiFi сигнала: <font color='green'>очень хороший</font>"));
                    break;
                case 6:
                    jsonWriteStr(configSetupJson, F("signal"), F("Уровень WiFi сигнала: <font color='green'>отличный</font>"));
                    break;
            }
        },
        nullptr, true);
}
