#pragma once

#include <Arduino.h>

#include <OneWire.h>

struct OneBusItem {
   private:
    uint8_t _addr[8];

   public:
    OneBusItem(const uint8_t addr[8]) {
        uint8_t i = 0;
        while (i < 8) {
            _addr[i] = addr[i];
            i++;
        }
    }

    bool
    equals(const uint8_t addr[8]) const {
        uint8_t i = 0;
        while (i < 8) {
            if (_addr[i] != addr[i]) return false;
            i++;
        }
        return true;
    }

    const String getAddress() {
        String res;
        for (uint8_t i = 0; i < 8; i++) {
            if (_addr[i] < 16) {
                res += "0";
            }
            res += String(_addr[i], HEX);
            if (i < 7) res += " ";
        }
        return res;
    }

    const String getAddUrl() {
        String addrStr = getAddress();
        addrStr.replace(" ", "");
        String res;
        res += "<a href='/?set.device'>";
        res += getFamily();
        res += "</a>";
        return res;
    }

    const String getFamily() {
        String res;
        switch (_addr[0]) {
            case 0x10:
                res = F("DS1820(S)");
                break;
            case 0x28:
                res = F("DS18B20");
            case 0x22:
                break;
                res = F("DS1822");
            case 0x3B:
                break;
                res = F("DS1825");
            case 0x42:
                break;
                res = F("DS28EA00");
            default:
                res = F("-");
        }
        return res;
    }
};

class OneWireBus {
   public:
    OneWireBus();
    ~OneWireBus();
    bool attached();
    void attach(uint8_t _pin);
    OneWire* get();
    void addItem(const uint8_t addr[8]);
    const String asJson();

   private:
    uint8_t _pin;
    OneWire* _bus;
    std::vector<OneBusItem> _items;
};

extern OneWireBus onewire;
