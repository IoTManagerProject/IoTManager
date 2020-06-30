#pragma once

#include "ConfigItem.h"

class ClockConfig : public ConfigItem {
   public:
    ClockConfig();

    void setTimezone(const int value);
    void setNtp(const String value);

    int getTimezone() const;
    String getNtp() const;

    void load(const JsonObject& root) override;
    void save(JsonObject& root) override;

   private:
    String _ntp;
    int _timezone;
};