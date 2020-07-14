#pragma once

#include <Arduino.h>
#include <list>

#include "Objects/LogEntry.h"

class LogBuffer {
   private:
    std::list<LogEntry> _pool;

   public:
    void push(const LogEntry& item) {
        _pool.push_back(item);
    }

    bool pop(LogEntry& item) {
        if (_pool.empty()) {
            return false;
        };
        item = _pool.front();
        _pool.pop_front();
        return true;
    }

    size_t size() {
        return _pool.size();
    }

    size_t available() {
        return !_pool.empty();
    };
};
