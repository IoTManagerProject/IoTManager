#include "Config/MqttConfig.h"

static const char* TAG_MQTT_SERVER = "mqttServer";
static const char* TAG_MQTT_PORT = "mqttPort";
static const char* TAG_MQTT_USER = "mqttUser";
static const char* TAG_MQTT_PASS = "mqttPass";
static const char* TAG_MQTT_PREFIX = "mqttPrefix";

MqttConfig::MqttConfig() {
    // TODO Defaults
}

void MqttConfig::setServer(const String value) {
    if (!_server.equals(value)) {
        _server = value;
        setChanged();
    }
}

void MqttConfig::setPort(const int value) {
    if (_port != value) {
        _port = value;
        setChanged();
    }
}

void MqttConfig::setUser(const String value) {
    if (!_user.equals(value)) {
        _user = value;
        setChanged();
    }
}

void MqttConfig::setPass(const String value) {
    if (!_pass.equals(value)) {
        _pass = value;
        setChanged();
    }
}

void MqttConfig::setPrefix(const String value) {
    if (!_prefix.equals(value)) {
        _prefix = value;
        setChanged();
    }
}

const String MqttConfig::getServer() const {
    return _server;
}

int MqttConfig::getPort() const {
    return _port;
}

const String MqttConfig::getUser() const {
    return _user;
}

const String MqttConfig::getPass() const {
    return _pass;
}

const String MqttConfig::getPrefix() const {
    return _prefix;
};

bool MqttConfig::validate() {
    return !_server.isEmpty() && _port != 0;
}

void MqttConfig::load(const JsonObject& root) {
    _prefix = root[TAG_MQTT_PREFIX].as<String>();
    _server = root[TAG_MQTT_SERVER].as<String>();
    _port = root[TAG_MQTT_PORT];
    _user = root[TAG_MQTT_USER].as<String>();
    _pass = root[TAG_MQTT_PASS].as<String>();
};

void MqttConfig::save(JsonObject& root) {
    root[TAG_MQTT_PREFIX] = _prefix.c_str();
    root[TAG_MQTT_SERVER] = _server.c_str();
    root[TAG_MQTT_PORT] = _port;
    root[TAG_MQTT_USER] = _user.c_str();
    root[TAG_MQTT_PASS] = _pass.c_str();
}

bool MqttConfig::setParamByName(const String& param, const String& value) {
    bool handled = true;
    if (param.equals(TAG_MQTT_SERVER)) {
        setServer(value);
    } else if (param.equals(TAG_MQTT_PORT)) {
        setPort(value.toInt());
    } else if (param.equals(TAG_MQTT_PREFIX)) {
        setPrefix(value);
    } else if (param.equals(TAG_MQTT_USER)) {
        setUser(value);
    } else if (param.equals(TAG_MQTT_PASS)) {
        setPass(value);
    } else {
        handled = false;
    }
    return handled;
}