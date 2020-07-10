#pragma once

#include <Arduino.h>

class Value {
   public:
    Value(){};

    virtual int read() {
        return onRead();
    }

    virtual int onRead();

   protected:
    int _value;
};
