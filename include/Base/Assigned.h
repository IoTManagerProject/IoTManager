#pragma once

#include <Arduino.h>

class Assigned {
   public:
    Assigned(const char* assigned) {
        strlcpy(_assigned, assigned, sizeof(_assigned));
    }

    const char* assigned() {
        return _assigned;
    }

    uint8_t getPin() const {
        return String(_assigned).toInt();
    }

   private:
    char _assigned[8];
};
