#pragma once

#include <Arduino.h>

#include "Base/Item.h"

#include <functional>

#include <Bounce2.h>

class Switch : public Item {
   public:
    Switch(const String& name, uint8_t pin, uint16_t debounce) : Item{name, String(pin, DEC), String(0, DEC)}, _obj{nullptr} {
        _obj = new Bounce();
        _obj->interval(debounce);
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

typedef std::function<void(Switch*, uint8_t)> OnSwitchChangeState;

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
