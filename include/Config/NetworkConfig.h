#pragma once

#include "ConfigItem.h"

class NetworkConfig : public ConfigItem {
   public:
    NetworkConfig();

    const char* getName() const;
    const char* getIPAddr() const;
    uint8_t getMode() const;
    void getSSID(uint8_t, String&) const;
    void getPasswd(uint8_t, String&) const;
    void setMode(uint8_t mode);
    void setSSID(uint8_t mode, const String& str);
    void setPasswd(uint8_t mode, const String& str);

    void load(const JsonObject& root) override;
    void save(JsonObject& root) override;

   private:
    bool updateField(char*, const String&, size_t);

   private:
    uint8_t _mode;
    char _name[33];
    char _ap_ssid[33];
    char _ap_passwd[33];
    char _sta_ssid[33];
    char _sta_passwd[33];
};
