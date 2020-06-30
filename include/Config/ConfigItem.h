#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

class ConfigItem {
   public:
    ConfigItem(){};

    void setChanged() {
        _lastChanged = millis();
    }

    unsigned long getLastChanged() {
        return _lastChanged;
    }

    virtual bool validate() {
        return true;
    }

    virtual void load(const JsonObject& root) = 0;
    virtual void save(JsonObject& root) = 0;

   private:
    unsigned long _lastChanged;
};