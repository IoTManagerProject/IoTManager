#pragma once

#include <Arduino.h>

#include "CommonTypes.h"

enum BusScanner_t {
    BS_I2C,
    BS_ONE_WIRE
};

class BusScanner {
   public:
    BusScanner(String& result, size_t tries) : _tries{tries}, _result{&result} {}

    void addResult(const String message) {
        _result->concat(message);
    }
    
    void addResult(uint8_t addr, boolean last = true) {
        _result->concat("0x");
        if (addr < 16) {
            _result->concat("0");
        }
        _result->concat(String(addr, HEX));
        _result->concat(!last ? ", " : "");
    };

    void scan() {
        if (!syncScan() && _tries--) {
            syncScan();
        }
    }
    virtual boolean syncScan();

   private:
    size_t _tries;
    BusScanner_t _type;
    String* _result;
};

class I2CScanner : public BusScanner {
   public:
    I2CScanner(String& result) : BusScanner(result, 2){};

    virtual boolean syncScan() override;
};

class OneWireScanner : public BusScanner {
   public:
    OneWireScanner(String& result) : BusScanner(result, 1){};

    virtual boolean syncScan() override;
};

class BusScannerFactory {
   public:
    static const char* label(BusScanner_t type) {
        switch (type) {
            case BS_I2C:
                return I2C_TAG;
            case BS_ONE_WIRE:
                return ONE_WIRE_TAG;
            default:
                return "";
        }
    }

    static BusScanner* get(String& str, BusScanner_t type, size_t tries = 1) {
        switch (type) {
            case BS_I2C:
                return new I2CScanner(str);
            case BS_ONE_WIRE:
                return new OneWireScanner(str);
            default:
                return nullptr;
        }
    }
};
