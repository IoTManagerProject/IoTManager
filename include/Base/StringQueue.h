#pragma once

#include <Arduino.h>
#include "circular_queue/circular_queue.h"

class StringQueue {
   private:
    circular_queue<String> _pool{32};

   public:
    void push(const String item) {
        _pool.push(item);
    }

    String pop() {
        return _pool.pop();
    }

    size_t available() {
        return _pool.available();
    };

    size_t free() {
        return _pool.available_for_push();
    }
};
