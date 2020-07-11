#pragma once

#include <Arduino.h>

#include "CircularBuffer.h"

class StringQueue {
   private:
    CircularBuffer<String, 32> _pool;

   public:
    void push(const String item) {
        _pool.push(item);
    }

    bool pop(String& item) {
        return _pool.pop(item);
    }

    size_t available() {
        return _pool.size();
    };
};
