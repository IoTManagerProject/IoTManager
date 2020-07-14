#pragma once

#include <Arduino.h>

#include "Utils/StringUtils.h"

class Assigned {
   public:
    Assigned(const String& assign) {
        _assign = strdup(assign.c_str());
    }

    ~Assigned() {
        delete _assign;
    }

    char* getAssign() {
        return _assign;
    }

   private:
    char* _assign;
};

class PinAssigned {
   public:
    PinAssigned(Assigned* obj) : _obj{obj} {};

    uint8_t getPin() {
        return String(_obj->getAssign()).toInt();
    }

   private:
    Assigned* _obj;
};

class OneWireAddressAssigned {
   public:
    OneWireAddressAssigned(Assigned* obj) : _obj{obj} {};

    uint8_t* getAddress() {
        char* addr = _obj->getAssign();
        if (addr) {
            for (size_t i = 0; i < 8; i++) {
                char h = addr[i * 2];
                char l = addr[(i * 2) + 1];
                _addr[i] = decodeHex(h) * 16 + decodeHex(l);
            }
        };
        return &_addr[0];
    }

   private:
    uint8_t _addr[8];
    Assigned* _obj;
};