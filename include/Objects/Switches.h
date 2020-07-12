#pragma once

#include <Arduino.h>
#include <Bounce2.h>
#include <functional>

#include "Base/Item.h"
#include "Base/Assigned.h"

class Switch : public Item,
               public PinAssigned {
   public:
    Switch(const String& name, const String& assign) : Item{name, assign},
                                                       PinAssigned{this} {
        _obj = new Bounce();
    }

    void onAssign() override {
        _obj->attach(getPin(), INPUT);
    }

    void setDebounce(int value) {
        _obj->interval(value);
    }

    bool hasValue() override {
        return _obj->update();
    }

    const String onGetValue() override {
        return _obj->read() ? "1" : "0";
    }

   private:
    Bounce* _obj;
};

typedef std::function<void(Switch*)> OnSwitchChangeState;

class Switches {
   public:
    Switch* add(const String& name, const String& assign);
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
