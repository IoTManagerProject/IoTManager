#pragma once

#include <Arduino.h>

class Assigned {
   public:
    Assigned(const char* assigned);
    const char* assigned();
    uint8_t getPin() const;

   private:
    char _assigned[8];
};
