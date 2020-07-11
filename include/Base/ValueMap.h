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
        onRead();
        return _mapper ? _mapper->evaluate(_value) : _value;
    }

    virtual bool onRead() {
        return false;
    };

   private:
    Mapper* _mapper;
};