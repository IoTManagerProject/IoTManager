#pragma once

#include <Arduino.h>

class ValueMapper {
   public:
    ValueMapper(long min_value, long max_value, long min_mapped, long max_mapped) : _min_value{min_value}, _max_value{max_value}, _min_mapped{min_mapped}, _max_mapped{max_mapped} {};

    long transform(long value) {
        return map(value, _min_value, _max_value, _min_mapped, _max_mapped);
    }

   private:
    long _min_value, _max_value, _min_mapped, _max_mapped;
};