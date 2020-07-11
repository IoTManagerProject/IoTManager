#include "Consts.h"

#include <Arduino.h>

const char* DEVICE_LAYOUT_FILE = "layout.txt";
const char* DEVICE_RUNTIME_FILE = "runtime.json";
const char* DEVICE_CONFIG_FILE = "config.json";
const char* DEVICE_COMMAND_FILE = "dev_conf.txt";
const char* DEVICE_SCENARIO_FILE = "dev_scen.txt";

const char* TAG_INIT = "init";
const char* TAG_OPTIONS = "options";
const char* TAG_RUNTIME = "runtime";
const char* TAG_LAST_VERSION = "last_version";

static const char* message_type_str[NUM_BROADCAST_MESSAGES] = {"iot_device", "mqtt_settings"};
const char* getMessageType(BroadcastMessage_t type) {
    return message_type_str[type];
}

BroadcastMessage_t getMessageType(const char* str) {
    for (size_t i = 0; i < NUM_BROADCAST_MESSAGES; i++) {
        if (strcmp(str, message_type_str[i]) == 0) {
            return BroadcastMessage_t(i);
        }
    }
    return NUM_BROADCAST_MESSAGES;
}