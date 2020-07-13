#pragma once

#include <Arduino.h>

class Named {
   public:
    Named(const String& name) {
        _name = strdup(name.c_str());
    }

    ~Named() {
        delete _name;
    }

    const char* getName() const {
        return _name;
    }

   protected:
    char* _name;
};