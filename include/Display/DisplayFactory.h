#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>
#include <Wire.h>

class DisplayHarwareIntf {
   public:
    virtual ~DisplayHarwareIntf() = default;
    virtual bool can(const String& model) = 0;
    virtual U8G2* createDefault(void) = 0;
    virtual U8G2* create(const String& param) = 0;
};

class STHardware : public DisplayHarwareIntf {
   public:
    bool can(const String& model) override {
        return model.startsWith("ST");
    }

    U8G2* createDefault(void) override {
        return createInstance(D6, D7, D8, D4, D3);
    }

    U8G2* create(const String& param) override {
        StaticJsonBuffer<512> doc;
        JsonObject& obj = doc.parseObject(param);
        uint8_t clock = obj.containsKey("clock") ? obj["clock"].as<int>() : D6;
        uint8_t data = obj.containsKey("data") ? obj["data"].as<int>() : D7;
        uint8_t cs = obj.containsKey("cs") ? obj["cs"].as<int>() : D8;
        uint8_t dc = obj.containsKey("dc") ? obj["dc"].as<int>() : D4;
        uint8_t rst = obj.containsKey("rst") ? obj["rst"].as<int>() : D3;        
        return createInstance(clock, data, cs, dc, rst);
    }

   private:
    U8G2* createInstance(uint8_t clock, uint8_t data, uint8_t cs, uint8_t dc, uint8_t rst) {
        Serial.printf("ST %d, %d, %d, %d, %d\r\n", clock, data, cs, dc, rst);
        return new U8G2_ST7565_ERC12864_F_4W_SW_SPI(U8G2_R0, clock, data, cs, dc, rst);
    }
};

class SHHardware : public DisplayHarwareIntf {
   public:
    bool can(const String& model) override {
        return model.startsWith("SH");
    }

    U8G2* createDefault(void) override {
        return createInstance(U8X8_PIN_NONE);
    }

    U8G2* create(const String& param) override {
        StaticJsonBuffer<512> doc;
        JsonObject& obj = doc.parseObject(param);
        uint8_t pin = obj.containsKey("pin") ? obj["pin"].as<int>() : U8X8_PIN_NONE;
        return createInstance(pin);
    }

   private:
    U8G2* createInstance(uint8_t pin) {
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

    U8G2* createInstance(const String& model, const String& param) {
        for (auto entry : _item)
            if (entry->can(model))
                return param.isEmpty() ? entry->createDefault() : entry->create(param);

        return nullptr;
    }
};
