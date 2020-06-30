#include "WiFiManager.h"

#include "Global.h"

#include "MqttClient.h"

static const char* MODULE = "WiFi";

// Errors
int wifi_error = 0;
int mqtt_error = 0;
bool _connected = false;

void startSTAMode() {
    setLedStatus(LED_SLOW);

    String ssid = jsonReadStr(configSetupJson, "routerssid");
    String passwd = jsonReadStr(configSetupJson, "routerpass");
    pm.info("STA Mode: " + ssid);

    WiFi.mode(WIFI_STA);
    if (ssid.isEmpty() && passwd.isEmpty()) {
        WiFi.begin();
    } else {
        if (WiFi.begin(ssid.c_str(), passwd.c_str()) == WL_CONNECT_FAILED) {
            pm.error("on begin");
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
                pm.error("no network");
                keepConnecting = false;
            } break;
            case WL_CONNECTED: {
                String hostIpStr = WiFi.localIP().toString();
                pm.info("http://" + hostIpStr);
                jsonWriteStr(configSetupJson, "ip", hostIpStr);
                keepConnecting = false;
            } break;
            case WL_CONNECT_FAILED: {
                pm.error("check credentials");
                jsonWriteInt(configOptionJson, "pass_status", 1);
                keepConnecting = false;
            } break;
            default:
                break;
        }
    } while (keepConnecting && tries--);

    if (isNetworkActive()) {
        ts.add(
            WIFI_MQTT_CONNECTION_CHECK, MQTT_RECONNECT_INTERVAL,
            [&](void*) {
                if (isNetworkActive()) {
                    if (MqttClient::isConnected()) {
                        if (!_connected) {
                            setLedStatus(LED_OFF);
                            _connected = true;
                        }
                    } else {
                        setLedStatus(LED_FAST);
                        if (MqttClient::connect()) {
                            setLedStatus(LED_OFF);
                        }
                        if (!just_load) mqtt_error++;
                    }
                } else {
                    if (_connected) {
                        pm.error("connection lost");
                        _connected = false;
                    }
                    ts.remove(WIFI_MQTT_CONNECTION_CHECK);
                    wifi_error++;
                    startAPMode();
                }
            },
            nullptr, true);

        setLedStatus(LED_OFF);
    } else {
        pm.error("failed: " + String(connRes, DEC));
        startAPMode();
    };
}

bool startAPMode() {
    setLedStatus(LED_ON);
    String ssid = jsonReadStr(configSetupJson, "apssid");
    String passwd = jsonReadStr(configSetupJson, "appass");
    pm.info("AP Mode: " + ssid);

    WiFi.mode(WIFI_AP);

    WiFi.softAP(ssid.c_str(), passwd.c_str());
    String hostIpStr = WiFi.softAPIP().toString();
    pm.info("Host IP: " + hostIpStr);
    jsonWriteStr(configSetupJson, "ip", hostIpStr);

    ts.add(
        WIFI_SCAN, 10 * 1000,
        [&](void*) {
            String sta_ssid = jsonReadStr(configSetupJson, "routerssid");
            pm.info("scanning for " + sta_ssid);
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
    pm.info("scan result: " + String(n, DEC));
    if (n == -2) {
        // не было запущено, запускаем
        pm.info(String("start scanning"));
        // async, show_hidden
        WiFi.scanNetworks(true, false);
    } else if (n == -1) {
        // все еще выполняется
        pm.info(String("scanning in progress"));
    } else if (n == 0) {
        // не найдена ни одна сеть
        pm.info(String("no networks found"));
        WiFi.scanNetworks(true, false);
    } else if (n > 0) {
        for (int8_t i = 0; i < n; i++) {
            if (WiFi.SSID(i) == ssid) {
                res = true;
            }
            pm.info((res ? "*" : "") + String(i, DEC) + ") " + WiFi.SSID(i));
        }
    }
    return res;
}

boolean isNetworkActive() {
    return WiFi.status() == WL_CONNECTED;
}
