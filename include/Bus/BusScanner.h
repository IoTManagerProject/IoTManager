#pragma once

#include <Arduino.h>

#include "Utils/JsonUtils.h"

class BusScanner {
   public:
    BusScanner(const char* tag, size_t tries) : _tag{tag}, _found{0}, _tries{tries}, _res{} {}

    void scan(String& out) {
        init();
        bool res;
        do {
            res = syncScan();
        } while (!res && --_tries);

        if (!_found) {
            _res = "не найдено";
        }
        
        jsonWriteStr(out, _tag, _res);
    }

   protected:
    virtual void init(){};

    virtual boolean syncScan() = 0;

   protected:
    void addResult(const String str) {
        _res += str;
    }

    void addResult(uint8_t addr[8]) {
        _found++;
        String str = "";
        for (size_t i = 0; i < 8; i++) {
            str += String(addr[i], HEX);
            str += i < 7 ? " " : ", ";
        }
        addResult(str);
    }

    void addResult(uint8_t addr, boolean last = true) {
        _found++;
        String str = "0x";
        if (addr < 16) {
            str += "0";
        }
        str += String(addr, HEX);
        str += !last ? ", " : ", ";
        addResult(str);
    };

   private:
    String _tag;
    size_t _found;
    size_t _tries;
    String _res;
};
