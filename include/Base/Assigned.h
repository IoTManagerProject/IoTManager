#pragma once

#include <Arduino.h>

#include "Utils/StringUtils.h"

class Assigned {
   public:
    Assigned() : _assign{NULL} {}

    ~Assigned() {
        if (_assign) {
            delete _assign;
        };
    }

    void setAssign(String assign) {
        if (_assign) {
            delete _assign;
        };
        _assign = strdup(assign.c_str());
        onAssign();
    }

    char* getAssign() {
        return _assign;
    }

   protected:
    virtual void onAssign() = 0;

   private:
    char* _assign;
};

class PinAssigned {
   public:
    PinAssigned(Assigned* obj) : _obj{obj} {};

    uint8_t getPin() {
        return atoi(_obj->getAssign());
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