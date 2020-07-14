#pragma once

#include "ConfigItem.h"

class MqttConfig : public ConfigItem {
   public:
    MqttConfig();

    void enable(bool enable = true);
    bool isEnabled();
    bool isValid();

    void setPrefix(const String value);
    void setServer(const String value);
    void setPort(const int value);
    void setUser(const String value);
    void setPass(const String value);

    const String getPrefix() const;
    const String getServer() const;
    int getPort() const;
    const String getUser() const;
    const String getPass() const;

    bool setParamByName(const String& param, const String& value) override;
    void load(const JsonObject& root) override;
    void save(JsonObject& root) override;

   private:
    bool _enabled;
    String _prefix;
    String _server;
    int _port;
    String _user;
    String _pass;
};