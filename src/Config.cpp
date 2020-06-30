#include "Config.h"

Config config;

Config::Config() : _items{
                       new GeneralConfig(),
                       new ClockConfig(),
                       new NetworkConfig(),
                       new MqttConfig()} {};

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

void Config::load(const String& str) {
    DynamicJsonBuffer buf;
    JsonObject& root = buf.parseObject(str);

    // "pushingboxid" : "v7C133E426B0C69E"
    // "weblogin" : "admin", "webpass" : "admin",
    // "udponoff" : "0",
    // "blink" : "1",
    // "oneWirePin" : "3",
    // "firmware_version" : "2.3.5", "ip" : "192.168.1.216", "uptime" : "00:00:28"

    for (size_t i = 0; i < NUM_CONFIGS; i++) {
        _items[i]->load(root);
    }

    _changed = millis();
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
