#include "Config/GeneralConfig.h"

static const char* TAG_SCENARIO_ENABLED = "scen";
static const char* TAG_BROADCAST_ENABLED = "udponoff";

GeneralConfig::GeneralConfig() {}

boolean GeneralConfig::isBroadcastEnabled() {
    return _broadcastEnabled;
}

void GeneralConfig::enableBroadcast(bool value) {
    bool changed = _broadcastEnabled != value;
    if (changed) {
        _broadcastEnabled = value;
        setChanged();
    }
}

boolean GeneralConfig::isScenarioEnabled() {
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
    _scenarioEnabled = root[TAG_SCENARIO_ENABLED] | true;
    _broadcastEnabled = root[TAG_BROADCAST_ENABLED] | true;
}

void GeneralConfig::save(JsonObject& root) {
    root[TAG_SCENARIO_ENABLED] = _scenarioEnabled;
    root[TAG_BROADCAST_ENABLED] = _broadcastEnabled;
};
