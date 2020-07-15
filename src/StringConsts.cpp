#include "StringConsts.h"

#include "Consts.h"

#include "Arduino.h"

const char* DEVICE_RUNTIME_FILE = "runtime.json";
const char* DEVICE_CONFIG_FILE = "config.json";
const char* DEVICE_COMMAND_FILE = "dev_conf.txt";
const char* DEVICE_SCENARIO_FILE = "dev_scen.txt";

const char* TAG_INIT = "init";
const char* TAG_OPTIONS = "options";
const char* TAG_RUNTIME = "runtime";
const char* TAG_LAST_VERSION = "last_version";

const char* TAG_ONE_WIRE = "oneWire";
const char* TAG_I2C = "i2c";
const char* TAG_SHARE_MQTT = "share_mqtt";
const char* TAG_CHECK_MQTT = "check_mqtt";

const char* TAG_BUTTON = "button";
const char* TAG_TIMER = "timer";
const char* TAG_PWM = "pwm";

const char* strBoolEnabled(bool value) {
    return value ? "enabled" : "disabled";
}

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