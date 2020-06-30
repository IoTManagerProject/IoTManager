#pragma once

#include "ConfigItem.h"

class MqttConfig : public ConfigItem {
   public:
    MqttConfig();

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

    void load(const JsonObject& root) override;
    void save(JsonObject& root) override;
    bool validate() override;

   private:
    String _prefix;
    String _server;
    int _port;
    String _user;
    String _pass;
};