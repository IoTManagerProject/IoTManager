#include "Utils/WiFiUtils.h"

void routerConnect() {
    setLedStatus(LED_SLOW);
    WiFi.mode(WIFI_STA);
    byte tries = 20;

    String _ssid = jsonReadStr(configSetupJson, "routerssid");
    String _password = jsonReadStr(configSetupJson, "routerpass");
    //WiFi.persistent(false);

    if (_ssid == "" && _password == "") {
        WiFi.begin();
    }
    else {
        WiFi.begin(_ssid.c_str(), _password.c_str());
        WiFi.setOutputPower(20.5);
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
    }
    else {
        Serial.println("");
        SerialPrint("I", "WIFI", "http://" + WiFi.localIP().toString());
        jsonWriteStr(configSetupJson, "ip", WiFi.localIP().toString());
        setLedStatus(LED_OFF);
        mqttInit();
    }
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

String RSSIquality() {
    String res = "not connected";
    if (WiFi.status() == WL_CONNECTED) {
        int rssi = WiFi.RSSI();
        if (rssi >= -50) {
            res = "Excellent";
        }
        else if (rssi < -50 && rssi >= -60) {
            res = "Very good";
        }
        else if (rssi < -60 && rssi >= -70) {
            res = "Good";
        }
        else if (rssi < -70 && rssi >= -80) {
            res = "Low";
        }
        else if (rssi < -80 && rssi > -100) {
            res = "Very low";
        }
        else if (rssi <= -100) {
            res = "No signal";
        }    
    } 
    return res;
}


