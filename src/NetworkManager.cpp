#include "NetworkManager.h"

#include "Global.h"

#include "Utils/TimeUtils.h"

namespace NetworkManager {

static const char* MODULE = "Network";

enum ConnectionStatus_t {
    CONNECTION_ERROR,
    CONNECTION_OK,
};

int wifi_error = 0;
int mqtt_error = 0;
bool _lastStatus = false;

void init() {
    pm.info("init");
#ifdef ESP8266
    WiFi.hostname(config.network()->getHostname());
#else
#endif
    WiFiMode_t mode = (WiFiMode_t)config.network()->getMode();

    switch (mode) {
        case WIFI_AP:
            startAPMode();
            break;
        case WIFI_STA:
            startSTAMode();
            break;
        default:
            break;
    }
}

boolean isNetworkActive() {
    return WiFi.status() == WL_CONNECTED;
}

IPAddress getHostIP() {
    return WiFi.getMode() == WIFI_STA ? WiFi.localIP() : WiFi.localIP();
}

void check_network_status() {
    LedStatus_t _led = LED_OFF;
    if (isNetworkActive()) {
        if (_lastStatus == CONNECTION_ERROR) {
            _lastStatus = CONNECTION_OK;
        }
        if (config.mqtt()->isEnabled()) {
            if (!MqttClient::isConnected()) {
                _led = LED_SLOW;
            }
        }
    } else {
        if (_lastStatus == CONNECTION_OK) {
            pm.error("connection lost");
            _led = LED_FAST;
            _lastStatus = CONNECTION_ERROR;
        }
    }
    setLedStatus(_led);
}

void startSTAMode() {
    ts.remove(WIFI_SCAN);

    String ssid, passwd;
    config.network()->getSSID(WIFI_STA, ssid);
    config.network()->getPasswd(WIFI_STA, passwd);

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
    int connRes;
    do {
        connRes = WiFi.waitForConnectResult();
        switch (connRes) {
            case -1: {
                pm.error("on timeout");
            } break;
            case WL_NO_SSID_AVAIL: {
                pm.error("no network");
                keepConnecting = false;
            } break;
            case WL_CONNECTED: {
                String hostIpStr = WiFi.localIP().toString();
                pm.info("http://" + hostIpStr);
                runtime.write("ip", hostIpStr);
                keepConnecting = false;
            } break;
            case WL_CONNECT_FAILED: {
                pm.error("check credentials");
                keepConnecting = false;
            } break;
            default:
                break;
        }
    } while (keepConnecting && tries--);

    if (isNetworkActive()) {
        ts.add(
            NETWORK_CONNECTION, ONE_SECOND_ms, [&](void*) {
                check_network_status();
            },
            nullptr, false);
    }
}

bool startAPMode() {
    setLedStatus(LED_ON);
    String ssid, passwd;
    config.network()->getSSID(WIFI_AP, ssid);
    config.network()->getPasswd(WIFI_AP, passwd);
    pm.info("AP Mode: " + ssid);

    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid.c_str(), passwd.c_str());
    String hostIpStr = WiFi.softAPIP().toString();
    pm.info("http://" + hostIpStr);

    runtime.write("ip", hostIpStr.c_str());

    ts.add(
        WIFI_SCAN, 10 * ONE_SECOND_ms,
        [&](void*) {
            if (WiFi.scanComplete()) {
                startScaninng();
            }
        },
        nullptr, true);
    return true;
}

void startScaninng() {
    WiFi.scanNetworksAsync([](int n) {
        if (!n) {
            pm.info(String("no networks found"));
        }
        String ssid;
        config.network()->getSSID(WIFI_STA, ssid);
        pm.info("found: " + String(n, DEC));
        bool res = false;
        for (int8_t i = 0; i < n; i++) {
            if (ssid == WiFi.SSID(i)) {
                res = true;
            }
            pm.info((res ? "*" : "") + String(i, DEC) + ") " + WiFi.SSID(i));
        }
        if (res) startSTAMode();
    },
                           true);
}

}  // namespace NetworkManager