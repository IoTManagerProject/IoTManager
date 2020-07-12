#pragma once

#include "Item.h"

class BaseSensor : public Item,
                   public PinAssigned,
                   public ValueMap {
   public:
    BaseSensor(const String& name, const String& assign) : Item{name, assign},
                                                           PinAssigned{this},
                                                           ValueMap{this} {}
    const String onGetValue() override {
        int raw = onReadSensor();
        int mapped = mapValue(raw);
        return String(mapped, DEC);
    }

    virtual int onReadSensor() = 0;
};
