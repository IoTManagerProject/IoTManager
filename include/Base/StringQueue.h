#pragma once

#include <Arduino.h>

#include <list>
class StringQueue {
   private:
    std::list<String> _pool;

   public:
    void push(const String& item) {
        _pool.push_back(item);
    }

    bool pop(String& item) {
        if (_pool.empty()) {
            return false;
        };
        item = _pool.front();
        _pool.pop_front();
        return true;
    }

    size_t available() {
        return !_pool.empty();
    };
};
