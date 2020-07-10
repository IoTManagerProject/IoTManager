#pragma once

#include <Arduino.h>

class Mapper {
   public:
    Mapper(long in_min, long in_max, long out_min, long out_max) : _in_min{in_min},
                                                                   _in_max{in_max},
                                                                   _out_min{out_min},
                                                                   _out_max{out_max} {}

    int evaluate(int value) {
        return map(value, _in_min, _in_max, _out_min, _out_max);
    }

   private:
    long _in_min, _in_max, _out_min, _out_max;
};
