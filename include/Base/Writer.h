#pragma once

class Writer {
   public:
    virtual ~Writer(){};
    virtual bool begin(unsigned int length) = 0;
    virtual bool write(const char* data) = 0;
    virtual bool end() = 0;
};
