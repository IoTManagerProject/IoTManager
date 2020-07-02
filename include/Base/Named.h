#pragma once

#include <Arduino.h>

class Named {
   public:
    Named(const char* name) {
        strlcpy(_name, name, sizeof(_name));
    }

    const char* getName() const {
        return _name;
    }

   protected:
    char _name[32];
};