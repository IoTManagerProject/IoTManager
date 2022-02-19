#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>
#include <Wire.h>

#include "DisplaySettings.h"

struct DisplayHardwareSettings {
    String type;
    String connection;
};

class DisplayHarwareIntf {
   public:
    virtual ~DisplayHarwareIntf() = default;
    virtual bool can(const DisplayHardwareSettings&) = 0;
    virtual U8G2* create(const DisplayHardwareSettings&) = 0;
};

class STHardware : public DisplayHarwareIntf {
   public:
    bool can(const DisplayHardwareSettings& s) override {
        return s.type.startsWith("ST");
    }

    U8G2* create(const DisplayHardwareSettings& s) override {
        StaticJsonBuffer<512> doc;

        JsonObject& obj = doc.parseObject(s.connection);
        uint8_t clock = obj.containsKey("clock") ? obj["clock"].as<int>() : D6;
        uint8_t data = obj.containsKey("data") ? obj["data"].as<int>() : D7;
        uint8_t cs = obj.containsKey("cs") ? obj["cs"].as<int>() : D8;
        uint8_t dc = obj.containsKey("dc") ? obj["dc"].as<int>() : D4;
        uint8_t rst = obj.containsKey("rst") ? obj["rst"].as<int>() : D3;

        return new U8G2_ST7565_ERC12864_F_4W_SW_SPI(U8G2_R0, clock, data, cs, dc, rst);
    }
};

class SHHardware : public DisplayHarwareIntf {
   public:
    bool can(const DisplayHardwareSettings& s) override {
        return s.type.startsWith("SH");
    }

    U8G2* create(const DisplayHardwareSettings& s) override {
        StaticJsonBuffer<512> doc;

        JsonObject& obj = doc.parseObject(s.connection);
        uint8_t pin = obj.containsKey("pin") ? obj["pin"].as<int>() : U8X8_PIN_NONE;

        return new U8G2_SH1106_128X64_NONAME_F_HW_I2C(U8G2_R0, pin);
    }
};

class DisplayFactory {
    std::vector<DisplayHarwareIntf*> _item;

   public:
    DisplayFactory() {
        _item.push_back(new STHardware());
        _item.push_back(new SHHardware());
    }

    ~DisplayFactory() {
        for (auto entry : _item) delete entry;
    }

    U8G2* createInstance(DisplaySettings* s) {
        DisplayHardwareSettings hw = {s->getType(), s->getConnection()};
        for (auto entry : _item)
            if (entry->can(hw))
                return entry->create(hw);
        return nullptr;
    }
};
