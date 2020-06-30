#pragma once

#include "ConfigItem.h"

class GeneralConfig : public ConfigItem {
   public:
    GeneralConfig();

    void enableBroadcast(boolean enabled);
    boolean isBroadcastEnabled();

    void enableScenario(boolean enabled);
    boolean isScenarioEnabled();

    void load(const JsonObject&) override;
    void save(JsonObject&) override;

   private:
    bool _broadcastEnabled;
    bool _scenarioEnabled;
};
