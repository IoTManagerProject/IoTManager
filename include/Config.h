#pragma once

#include "Config/ConfigItem.h"
#include "Config/ClockConfig.h"
#include "Config/GeneralConfig.h"
#include "Config/NetworkConfig.h"
#include "Config/MqttConfig.h"
#include "Config/WebConfig.h"

enum Configs {
    CONFIG_GENERAL,
    CONFIG_CLOCK,
    CONFIG_NETWORK,
    CONFIG_MQTT,
    CONFIG_WEB,
    NUM_CONFIGS
};

class Config {
   public:
    Config();

    bool hasChanged();

    void load(const String&);
    void save(JsonObject& root);
    bool setParamByName(const String& param, const String& value);

    void save(String& str);
    const String append(String& str);

    MqttConfig* mqtt();
    ClockConfig* clock();
    NetworkConfig* network();
    GeneralConfig* general();
    WebConfig* web();

    void setSynced();

   private:
    ConfigItem* _items[NUM_CONFIGS];
    unsigned long _timestamp;
};

extern Config config;
