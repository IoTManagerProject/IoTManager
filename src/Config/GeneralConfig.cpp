#include "Config/GeneralConfig.h"

static const char* TAG_BROADCAST_NAME = "devname";
static const char* TAG_BROADCAST_ENABLED = "udponoff";
static const char* TAG_BROADCAST_INTERVAL = "udptime";
static const char* TAG_SCENARIO_ENABLED = "scen";
static const char* TAG_LED_ENABLED = "blink";
static const char* TAG_PUSHINGBOX_ID = "pushingboxid";

GeneralConfig::GeneralConfig() {}

void GeneralConfig::setBroadcastName(const String value) {
    if (!_broadcastName.equals(value)) {
        _broadcastName = value;
        setChanged();
    }
}

const String GeneralConfig::getBroadcastName() const {
    return _broadcastName;
}

unsigned long GeneralConfig::getBroadcastInterval() const {
    return _broadcastInterval;
}

void GeneralConfig::setBroadcastInterval(unsigned long value) {
    bool changed = _broadcastInterval != value;
    if (changed) {
        _broadcastInterval = value;
        setChanged();
    }
}

boolean GeneralConfig::isBroadcastEnabled() const {
    return _broadcastEnabled;
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
    _broadcastName = root[TAG_BROADCAST_NAME].as<String>();
    _broadcastEnabled = root[TAG_BROADCAST_ENABLED] | true;
    _broadcastInterval = root[TAG_BROADCAST_INTERVAL] | 60;
    _scenarioEnabled = root[TAG_SCENARIO_ENABLED] | true;
    _ledEnabled = root[TAG_LED_ENABLED] | true;
    _pushingBoxId = root[TAG_PUSHINGBOX_ID].as<String>();
}

void GeneralConfig::save(JsonObject& root) {
    root[TAG_BROADCAST_NAME] = _broadcastName.c_str();
    root[TAG_BROADCAST_ENABLED] = _broadcastEnabled;
    root[TAG_BROADCAST_INTERVAL] = _broadcastInterval;
    root[TAG_SCENARIO_ENABLED] = _scenarioEnabled;
    root[TAG_LED_ENABLED] = _ledEnabled;
    root[TAG_PUSHINGBOX_ID] = _pushingBoxId.c_str();
};

bool GeneralConfig::setParamByName(const String& param, const String& value) {
    bool handled = true;
    if (param.equals(TAG_BROADCAST_NAME)) {
        setBroadcastName(value);
    } else if (param.equals(TAG_BROADCAST_ENABLED)) {
        enableBroadcast(param.toInt());
    } else if (param.equals(TAG_BROADCAST_INTERVAL)) {
        setBroadcastInterval(param.toInt());
    } else if (param.equals(TAG_SCENARIO_ENABLED)) {
        enableScenario(value.toInt());
    } else if (param.equals(TAG_LED_ENABLED)) {
        enableLed(value.toInt());
    } else if (param.equals(TAG_PUSHINGBOX_ID)) {
        setPushingboxId(value);
    } else {
        handled = false;
    }
    return handled;
}