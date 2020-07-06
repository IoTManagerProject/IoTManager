#pragma once

#include <Arduino.h>

class StringQueue {
   private:
    std::vector<String> _pool;

   public:
    void push(const String item) {
        _pool.push_back(item);
    }

    void pop(String& item) {
        item = _pool.back();
        _pool.pop_back();
    }

    size_t available() {
        return _pool.size();
    };
};
