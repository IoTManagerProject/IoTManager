#pragma once

#include <Arduino.h>

class Writer {
   public:
    virtual ~Writer(){};

    virtual bool print(const char* data) = 0;

    virtual bool begin(size_t length) = 0;
    virtual bool write(const char* data) = 0;
    virtual bool end() = 0;
};
