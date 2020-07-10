#pragma once

#include <Arduino.h>

#include <Base/Value.h>
#include <Base/Mapper.h>

class ValueMap : public Value {
   public:
    ValueMap() : _mapper{NULL} {};

    void setMap(long in_min, long in_max, long out_min, long out_max) {
        _mapper = new Mapper{in_min, in_max, out_min, out_max};
    }

    int read() override {
        return _mapper ? _mapper->evaluate(onRead()) : Value::read();
    }

    virtual int onRead() = 0;

   private:
    Mapper* _mapper;
};