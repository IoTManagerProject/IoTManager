#include "Config/NetworkConfig.h"

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

static const char* TAG_WIFI_MODE = "mode";
static const char* TAG_HOSTNAME = "hostname";
static const char* TAG_AP_SSID = "apssid";
static const char* TAG_AP_PASSWD = "appass";
static const char* TAG_STA_SSID = "routerssid";
static const char* TAG_STA_PASSWD = "routerpass";

NetworkConfig::NetworkConfig(){
    // TODO Defaults
};

void NetworkConfig::setHostname(const String str) {
    bool changed = updateField(_hostname, str, sizeof(_hostname));
    if (changed) {
        setChanged();
    }
}

void NetworkConfig::setMode(uint8_t mode) {
    bool changed = _mode != mode;
    if (changed) {
        _mode = mode;
        setChanged();
    }
}

void NetworkConfig::setSSID(uint8_t mode, const String& str) {
    char* field = (mode == 1) ? &_sta_ssid[0] : &_ap_ssid[0];
    bool changed = updateField(field, str, 33);
    if (changed) {
        setChanged();
    }
}

void NetworkConfig::setPasswd(uint8_t mode, const String& str) {
    char* field = (mode == 1) ? &_sta_passwd[0] : &_ap_passwd[0];
    bool changed = updateField(field, str, 33);
    if (changed) {
        setChanged();
    }
}

uint8_t NetworkConfig::getMode() const {
    return _mode;
}

const char* NetworkConfig::getHostname() const {
    return _hostname;
}

void NetworkConfig::getSSID(uint8_t mode, String& str) const {
    str = (mode == 1) ? _sta_ssid : _ap_ssid;
}

void NetworkConfig::getPasswd(uint8_t mode, String& str) const {
    str = (mode == 1) ? _sta_passwd : _ap_passwd;
}

void NetworkConfig::load(const JsonObject& root) {
    _mode = root[TAG_WIFI_MODE] | 1;
    strlcpy(_hostname, root[TAG_HOSTNAME] | "", sizeof(_hostname));
    strlcpy(_ap_ssid, root[TAG_AP_SSID] | "", sizeof(_ap_ssid));
    strlcpy(_ap_passwd, root[TAG_AP_PASSWD] | "", sizeof(_ap_passwd));
    strlcpy(_sta_ssid, root[TAG_STA_SSID] | "", sizeof(_sta_ssid));
    strlcpy(_sta_passwd, root[TAG_STA_PASSWD] | "", sizeof(_sta_passwd));
}

void NetworkConfig::save(JsonObject& root) {
    root[TAG_WIFI_MODE] = _mode;
    root[TAG_HOSTNAME] = _hostname;
    root[TAG_AP_SSID] = _ap_ssid;
    root[TAG_AP_PASSWD] = _ap_passwd;
    root[TAG_STA_SSID] = _sta_ssid;
    root[TAG_STA_PASSWD] = _sta_passwd;
};

bool NetworkConfig::setParamByName(const String& param, const String& value) {
    bool handled = true;
    if (param.equals(TAG_WIFI_MODE)) {
        setMode(value.toInt());
    } else if (param.equals(TAG_HOSTNAME)) {
        setHostname(value);
    } else if (param.equals(TAG_STA_SSID)) {
        setSSID(WIFI_STA, value);
    } else if (param.equals(TAG_STA_PASSWD)) {
        setPasswd(WIFI_STA, value);
    } else if (param.equals(TAG_AP_SSID)) {
        setSSID(WIFI_AP, value);
    } else if (param.equals(TAG_AP_PASSWD)) {
        setPasswd(WIFI_AP, value);
    } else {
        handled = false;
    }
    return handled;
}