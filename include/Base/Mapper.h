#pragma once

#include <Arduino.h>

class Mapper {
   public:
    Mapper(long in_min, long in_max, long out_min, long out_max) : _constrain{true},
                                                                   _in_min{in_min},
                                                                    _in_max{in_max},
                                                                   _out_min{out_min},
                                                                   _out_max{out_max} {}

    int mapValue(int value) {
        value = _constrain ? constrain(value, _in_min, _in_max) : value;
        return map(value, _in_min, _in_max, _out_min, _out_max);
    }

   private:
    bool _constrain;
    long _in_min, _in_max, _out_min, _out_max;
};
