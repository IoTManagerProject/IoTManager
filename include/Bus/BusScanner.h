#pragma once

#include <Arduino.h>

class BusScanner {
   public:
    BusScanner(const char* tag, String& out, size_t tries) : _found{0},
                                                             _tries{tries},
                                                             _out{&out} {
        _tag = new char(strlen(tag) + 1);
        strcpy(_tag, tag);
    }

    void scan() {
        init();
        bool res;
        do {
            res = syncScan();
        } while (!res && _tries);

        if (!_found) {
            addResult("не найдено");
        }
    }

    const char* tag() {
        return _tag;
    }

   protected:
    virtual void init(){};

    virtual boolean syncScan() = 0;

   protected:
    void addResult(const String& str) {
        _out->concat(str);
    }

    void addResult(uint8_t addr, boolean last = true) {
        _found++;
        String str = "0x";
        if (addr < 16) {
            str += "0";
        }
        str += String(addr, HEX);
        str += !last ? ", " : "";
        addResult(str);
    };

   private:
    char* _tag;
    size_t _found;
    size_t _tries;
    String* _out;
};
