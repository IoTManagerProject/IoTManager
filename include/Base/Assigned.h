#pragma once

#include <Arduino.h>

class Assigned {
   public:
    Assigned(const String& assign) {
        strlcpy(_assign, assign.c_str(), sizeof(_assign));
        onAssign();
    }

    virtual void onAssign() {}

    const char* getAssign() {
        return _assign;
    }

   protected:
    char _assign[8];
};

class PinAssigned : public Assigned {
   public:
    PinAssigned(const String& assign) : Assigned(assign){};

    uint8_t getPin() {
        return atoi(_assign);
    }
};
