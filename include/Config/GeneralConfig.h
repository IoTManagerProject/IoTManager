#pragma once

#include "ConfigItem.h"

class GeneralConfig : public ConfigItem {
   public:
    GeneralConfig();

    void enableLed(bool value);
    boolean isLedEnabled();

    void enableBroadcast(boolean value);
    boolean isBroadcastEnabled();

    void enableScenario(boolean value);
    boolean isScenarioEnabled();

    void load(const JsonObject&) override;
    void save(JsonObject&) override;

   private:
    bool _broadcastEnabled;
    bool _scenarioEnabled;
    bool _ledEnabled;
};
