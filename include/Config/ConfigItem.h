#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

class ConfigItem {
   public:
    ConfigItem() : _timestamp{0} {};

    void setChanged(unsigned long timestamp = millis()) {
        _timestamp = timestamp;
    }

    unsigned long getTimestamp() {
        return _timestamp;
    }

    virtual void loadString(const String& str) {
        DynamicJsonBuffer buf;
        JsonObject& root = buf.parseObject(str);
        load(root);
    }

    virtual bool setParamByName(const String& param, const String& value) {
        return false;
    }

    virtual void load(const JsonObject& root) = 0;

    virtual void save(JsonObject& root) = 0;

   protected:
    virtual bool updateField(char* prev, const String& next, size_t size) {
        bool changed = !next.equals(prev);
        if (changed) {
            strlcpy(prev, next.c_str(), size);
        }
        return changed;
    }

   private:
    unsigned long _timestamp;
};