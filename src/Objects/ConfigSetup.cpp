#include "Objects/ConfigSetup.h"

ConfigSetup configSetup;

static const char* TAG_WIFI_MODE = "mode";
static const char* TAG_HOSTNAME = "name";
static const char* TAG_AP_SSID = "apssid";
static const char* TAG_AP_PASSWD = "appass";
static const char* TAG_STA_SSID = "routerssid";
static const char* TAG_STA_PASSWD = "routerpass";
static const char* TAG_NTP = "ntp";
static const char* TAG_TIMEZONE = "timezone";
static const char* TAG_SCENARIO_ENABLED = "scen";
static const char* TAG_BROADCAST_ENABLED = "udponoff";

boolean ConfigSetup::isBroadcastEnabled() {
    return _broadcastEnabled;
}

void ConfigSetup::enableBroadcast(bool value) {
    _broadcastEnabled = value;
}

void ConfigSetup::setChanged(boolean value) {
    if (value) _changed = value;
}

void ConfigSetup::setMode(uint8_t mode) {
    bool changed = _mode != mode;
    if (changed) {
        _mode = mode;
    }
    setChanged(changed);
}

boolean ConfigSetup::isScenarioEnabled() {
    return _scenarioEnabled;
}

void ConfigSetup::enableScenario(boolean enable) {
    bool changed = _scenarioEnabled != enable;
    if (changed) {
        _scenarioEnabled = enable;
    }
    setChanged(changed);
}

void ConfigSetup::setSSID(uint8_t mode, const String& str) {
    char* field = (mode == 1) ? &_sta_ssid[0] : &_ap_ssid[0];
    bool changed = updateField(field, str, 33);
    setChanged(changed);
}

void ConfigSetup::setPasswd(uint8_t mode, const String& str) {
    char* field = (mode == 1) ? &_sta_passwd[0] : &_ap_passwd[0];
    bool changed = updateField(field, str, 33);
    setChanged(changed);
}

bool ConfigSetup::updateField(char* prev, const String& next, size_t size) {
    bool changed = !next.equals(prev);
    if (changed) {
        strlcpy(prev, next.c_str(), size);
    }
    return changed;
}

uint8_t ConfigSetup::getMode() const {
    return _mode;
}

const char* ConfigSetup::getName() const {
    return _name;
}

void ConfigSetup::getSSID(uint8_t mode, String& str) const {
    str = (mode == 1) ? _sta_ssid : _ap_ssid;
}

void ConfigSetup::getPasswd(uint8_t mode, String& str) const {
    str = (mode == 1) ? _sta_passwd : _ap_passwd;
}

void ConfigSetup::load(const String& str) {
    DynamicJsonBuffer buf;
    JsonObject& root = buf.parseObject(str);

    // "mqttServer" : "***REMOVED***", "mqttPort" : 1883, "mqttPrefix" : "/iot", "mqttUser" : "devices", "mqttPass" : "devices", "scen" : "1", "pushingboxid" : "v7C133E426B0C69E", "weblogin" : "admin", "webpass" : "admin", "udponoff" : "0", "blink" : "1", "oneWirePin" : "3", "firmware_version" : "2.3.5", "ip" : "192.168.1.216", "uptime" : "00:00:28"

    _mode = root[TAG_WIFI_MODE] | 2;
    strlcpy(_name, root[TAG_HOSTNAME], sizeof(_name));
    strlcpy(_ap_ssid, root[TAG_AP_SSID], sizeof(_ap_ssid));
    strlcpy(_ap_passwd, root[TAG_AP_PASSWD], sizeof(_ap_passwd));
    strlcpy(_sta_ssid, root[TAG_STA_SSID], sizeof(_sta_ssid));
    strlcpy(_sta_passwd, root[TAG_STA_PASSWD], sizeof(_sta_passwd));
    strlcpy(_ntp, root[TAG_NTP], sizeof(_ntp));

    _timezone = root[TAG_TIMEZONE] | 0;
    _scenarioEnabled = root[TAG_SCENARIO_ENABLED] | true;
    _broadcastEnabled = root[TAG_BROADCAST_ENABLED] | true;

    _changed = false;
}

const String ConfigSetup::save(String& str) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(str);

    root[TAG_WIFI_MODE] = _mode;
    root[TAG_HOSTNAME] = _name;
    root[TAG_AP_SSID] = _ap_ssid;
    root[TAG_AP_PASSWD] = _ap_passwd;

    root[TAG_STA_SSID] = _sta_ssid;
    root[TAG_STA_PASSWD] = _sta_passwd;

    root[TAG_NTP] = _ntp;
    root[TAG_TIMEZONE] = _timezone;

    root[TAG_SCENARIO_ENABLED] = _scenarioEnabled;

    root[TAG_BROADCAST_ENABLED] = _broadcastEnabled;

    str = "";
    root.printTo(str);

    _changed = false;

    return str;
};
