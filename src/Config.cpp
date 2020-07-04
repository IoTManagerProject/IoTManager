#include "Config.h"

Config config;

Config::Config() : _items{
                       new GeneralConfig(),
                       new ClockConfig(),
                       new NetworkConfig(),
                       new MqttConfig(),
                       new WebConfig()} {};

GeneralConfig* Config::general() {
    return (GeneralConfig*)_items[CONFIG_GENERAL];
}

ClockConfig* Config::clock() {
    return (ClockConfig*)_items[CONFIG_CLOCK];
}

NetworkConfig* Config::network() {
    return (NetworkConfig*)_items[CONFIG_NETWORK];
}

MqttConfig* Config::mqtt() {
    return (MqttConfig*)_items[CONFIG_MQTT];
}

WebConfig* Config::web() {
    return (WebConfig*)_items[CONFIG_WEB];
}

void Config::setSynced() {
    unsigned long timestamp = millis();
    for (size_t i = 0; i < NUM_CONFIGS; i++) {
        _items[i]->setChanged(timestamp);
    }
    _timestamp = timestamp;
}

bool Config::hasChanged() {
    for (size_t i = 0; i < NUM_CONFIGS; i++) {
        if (_timestamp < _items[i]->getTimestamp()) return true;
    }
    return false;
}

void Config::load(const String& str) {
    DynamicJsonBuffer buf;
    JsonObject& root = buf.parseObject(str);

    for (size_t i = 0; i < NUM_CONFIGS; i++) {
        _items[i]->load(root);
    }

    _timestamp = millis();
}

bool Config::setParamByName(const String& param, const String& value) {
    bool handled = false;
    for (size_t i = 0; i < NUM_CONFIGS; i++) {
        if (_items[i]->setParamByName(param, value)) {
            handled = true;
            break;
        }
    }
    return handled;
};

const String Config::append(String& str) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(str);
    save(root);
    str = "";
    root.printTo(str);
    return str;
};

void Config::save(String& str) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    save(root);
    root.printTo(str);
};

void Config::save(JsonObject& root) {
    for (size_t i = 0; i < NUM_CONFIGS; i++) {
        _items[i]->save(root);
    }
};
