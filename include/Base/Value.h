#pragma once

#include <Arduino.h>

class Value {
   public:
    Value(long value) : _value{value} {};

    long value() {
        return _value;
    }

   protected:
    long _value;
};
