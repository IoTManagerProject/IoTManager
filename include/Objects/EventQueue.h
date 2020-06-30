#pragma once

#include <Arduino.h>

class EventQueue {
   private:
    std::vector<String> _pool;

   public:
    void push(const String item);

    const String pop();

    size_t available();
};

extern EventQueue events;
