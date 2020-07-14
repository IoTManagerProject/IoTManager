#include "StringConsts.h"

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

const char* strBoolEnabled(bool value) {
    return value ? "enabled" : "disabled";
}