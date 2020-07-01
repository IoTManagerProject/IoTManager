#pragma once

#include "ConfigItem.h"

class WebConfig : public ConfigItem {
   public:
    WebConfig();

    const String getLogin() const;
    void setLogin(const String);

    const String getPass() const;
    void setPass(const String);

    void load(const JsonObject&) override;
    void save(JsonObject&) override;

   private:
    char _login[33];
    char _pass[33];
};
