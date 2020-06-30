#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

#include "Consts.h"

struct ConfigSetup {
   public:
    ConfigSetup(){};

    const char* getName() const;
    const char* getIPAddr() const;
    uint8_t getMode() const;
    void getSSID(uint8_t, String&) const;
    void getPasswd(uint8_t, String&) const;
    void setMode(uint8_t mode);
    void setSSID(uint8_t mode, const String& str);
    void setPasswd(uint8_t mode, const String& str);

    void enableBroadcast(boolean enabled);
    boolean isBroadcastEnabled();

    void enableScenario(boolean enabled);
    boolean isScenarioEnabled();

    void load(const String&);
    const String save(String& str);

   private:
    void setChanged(boolean = true);
    bool updateField(char*, const String&, size_t);

   private:
    uint8_t _mode;
    char _name[33];
    char _ap_ssid[33];
    char _ap_passwd[33];
    char _sta_ssid[33];
    char _sta_passwd[33];

    int _timezone;
    char _ntp[33];

    bool _broadcastEnabled;
    bool _scenarioEnabled;

   private:
    bool _changed;
};

extern ConfigSetup configSetup;