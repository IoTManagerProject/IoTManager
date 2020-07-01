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

void Config::load(const String& str) {
    DynamicJsonBuffer buf;
    JsonObject& root = buf.parseObject(str);

    // "pushingboxid" : "v7C133E426B0C69E"
    // "oneWirePin" : "3",
    // "firmware_version" : "2.3.5",
    // "ip" : "192.168.1.216",
    // "uptime" : "00:00:28"

    for (size_t i = 0; i < NUM_CONFIGS; i++) {
        _items[i]->load(root);
    }

    _timestamp = millis();
}

const String Config::save(String& str) {
    DynamicJsonBuffer jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(str);

    for (size_t i = 0; i < NUM_CONFIGS; i++) {
        _items[i]->save(root);
    }

    str = "";
    root.printTo(str);
    return str;
};
