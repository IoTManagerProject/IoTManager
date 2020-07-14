#pragma once

#include "Item.h"

class BaseSensor : public Item,
                   public Value {
   public:
    BaseSensor(const String& name, const String& assign, const ValueType_t type) : Item{name, assign},
                                                                                   Value{type} {}

    virtual const String onReadSensor() = 0;

   protected:
    const String onGetValue() override {
        return onReadSensor();
    }
};
