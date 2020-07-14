#pragma once

#include "ConfigItem.h"

class GeneralConfig : public ConfigItem {
   public:
    GeneralConfig();

    const String getUpdateUrl() const;
    void setUpdateUrl(const String value);

    const String getBroadcastName() const;
    void setBroadcastName(const String value);

    const String getPushingboxId() const;
    void setPushingboxId(String value);

    void enableLed(bool value);
    boolean isLedEnabled() const;

    unsigned long getBroadcastInterval() const;
    void setBroadcastInterval(int value);

    void enableBroadcast(boolean value);
    boolean isBroadcastEnabled() const;

    void enableScenario(boolean value);
    boolean isScenarioEnabled() const;

    bool setParamByName(const String& param, const String& value) override;
    void load(const JsonObject&) override;
    void save(JsonObject&) override;

   private:
    String _updateUrl;
    String _broadcastName;
    String _pushingBoxId;
    int _broadcastInterval;
    bool _broadcastEnabled;
    bool _scenarioEnabled;
    bool _ledEnabled;
};
