#pragma once

#include "Config/ConfigItem.h"
#include "Config/ClockConfig.h"
#include "Config/GeneralConfig.h"
#include "Config/NetworkConfig.h"
#include "Config/MqttConfig.h"

enum Configs {
    CONFIG_GENERAL,
    CONFIG_CLOCK,
    CONFIG_NETWORK,
    CONFIG_MQTT,
    NUM_CONFIGS
};

class Config {
   public:
    Config();

    void load(const String&);
    const String save(String& str);

    MqttConfig* mqtt();
    ClockConfig* clock();
    NetworkConfig* network();
    GeneralConfig* general();

   private:
    ConfigItem* _items[NUM_CONFIGS];

   private:
    unsigned long _changed;
};

extern Config config;
