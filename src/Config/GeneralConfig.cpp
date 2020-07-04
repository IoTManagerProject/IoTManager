#include "Config/GeneralConfig.h"

static const char* TAG_NAME = "name";
static const char* TAG_SCENARIO_ENABLED = "scen";
static const char* TAG_BROADCAST_ENABLED = "udponoff";
static const char* TAG_LED_ENABLED = "blink";
static const char* TAG_PUSHINGBOX_ID = "pushingboxid";

GeneralConfig::GeneralConfig() {}

void GeneralConfig::setName(const String value) {
    if (!_name.equals(value)) {
        _name = value;
        setChanged();
    }
}

const String GeneralConfig::getName() const {
    return _name;
}

const String GeneralConfig::getPushingboxId() const {
    return _pushingBoxId;
}

void GeneralConfig::setPushingboxId(String value) {
    if (!_pushingBoxId.equals(value)) {
        _pushingBoxId = value;
        setChanged();
    }
}

boolean GeneralConfig::isLedEnabled() const {
    return _ledEnabled;
}

void GeneralConfig::enableLed(bool value) {
    bool changed = _ledEnabled != value;
    if (changed) {
        _ledEnabled = value;
        setChanged();
    }
}

boolean GeneralConfig::isBroadcastEnabled() const {
    return _broadcastEnabled;
}

void GeneralConfig::enableBroadcast(bool value) {
    bool changed = _broadcastEnabled != value;
    if (changed) {
        _broadcastEnabled = value;
        setChanged();
    }
}

boolean GeneralConfig::isScenarioEnabled() const {
    return _scenarioEnabled;
}

void GeneralConfig::enableScenario(boolean value) {
    bool changed = _scenarioEnabled != value;
    if (changed) {
        _scenarioEnabled = value;
        setChanged();
    }
}

void GeneralConfig::load(const JsonObject& root) {
    _name = root[TAG_NAME].as<String>();
    _scenarioEnabled = root[TAG_SCENARIO_ENABLED] | true;
    _broadcastEnabled = root[TAG_BROADCAST_ENABLED] | true;
    _ledEnabled = root[TAG_LED_ENABLED] | true;
    _pushingBoxId = root[TAG_PUSHINGBOX_ID].as<String>();
}

void GeneralConfig::save(JsonObject& root) {
    root[TAG_NAME] = _name.c_str();
    root[TAG_SCENARIO_ENABLED] = _scenarioEnabled;
    root[TAG_BROADCAST_ENABLED] = _broadcastEnabled;
    root[TAG_LED_ENABLED] = _ledEnabled;
    root[TAG_PUSHINGBOX_ID] = _pushingBoxId.c_str();
};
