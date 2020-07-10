#pragma once

#include <Arduino.h>
#include <Bounce2.h>
#include <functional>

#include "Base/Item.h"

class Switch : public Item, public PinAssigned {
   public:
    Switch(const String& name, const String& assign, uint16_t debounce) : Item{name, assign, String(0, DEC)}, PinAssigned(assign) {
        _obj = new Bounce();
        _obj->interval(debounce);
    }

    void onPinAssign() override {
        _obj->attach(getPin(), INPUT);
        _state = _obj->read();
    }

    bool update() {
        bool res = false;
        if (_obj) {
            res = _obj->update();
            _state = _obj->read();
        }
        return res;
    }

    void onStateChange() override {
    }

   private:
    Bounce* _obj;
};

typedef std::function<void(Switch*)> OnSwitchChangeState;

class Switches {
   public:
    Switch* add(const String& name, const String& pin, const String& debounce);
    void loop();
    Switch* last();
    Switch* at(size_t index);
    Switch* get(const String name);
    size_t count();

    void setOnChangeState(OnSwitchChangeState);

   private:
    std::vector<Switch> _items;
    OnSwitchChangeState _onChange;
};

extern Switches mySwitches;
