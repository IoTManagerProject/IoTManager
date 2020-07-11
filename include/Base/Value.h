#pragma once

#include <Arduino.h>

class Value {
   public:
    Value(){};

    virtual int read() {
        onRead();
        return _value;
    }

    virtual bool onRead();

   protected:
    int _value;
};
