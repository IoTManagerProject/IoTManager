#pragma once

#include <Arduino.h>

class Readable {
   public:
    Readable();

    virtual int readInt() {
        return 0;
    }
};