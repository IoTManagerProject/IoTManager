#pragma once

#include <Arduino.h>

class StringQueue {
   private:
    std::vector<String> _pool;

   public:
    void push(const String item) {
        _pool.push_back(item);
    }

    const String pop() {
        String item = _pool.back();
        _pool.pop_back();
        return item;
    }

    size_t available() {
        return _pool.size();
    };
};
