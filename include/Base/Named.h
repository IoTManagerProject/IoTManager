#pragma once

#include <Arduino.h>

class Named {
   public:
    Named(const char* name);
    const char* name();

   private:
    char _name[32];
};