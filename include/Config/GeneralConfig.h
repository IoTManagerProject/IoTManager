#pragma once

#include "ConfigItem.h"

class GeneralConfig : public ConfigItem {
   public:
    GeneralConfig();
    
    const String getName() const;
    void setName(const String value);

    const String getPushingboxId() const;
    void setPushingboxId(String value);

    void enableLed(bool value);
    boolean isLedEnabled() const;

    void enableBroadcast(boolean value);
    boolean isBroadcastEnabled() const;

    void enableScenario(boolean value);
    boolean isScenarioEnabled() const;

    void load(const JsonObject&) override;
    void save(JsonObject&) override;

   private:
    String _name;
    String _pushingBoxId;
    bool _broadcastEnabled;
    bool _scenarioEnabled;
    bool _ledEnabled;
};
