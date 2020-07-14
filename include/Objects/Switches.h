#pragma once

#include <Arduino.h>
#include <functional>

#include <Bounce2.h>

#include "Base/Item.h"
#include "Base/Assigned.h"

class Switch : public Item,
               public Value,
               public PinAssigned {
   public:
    Switch(const String& name, const String& assign) : Item{name, assign},
                                                       Value{VT_INT},
                                                       PinAssigned{this} {
        _obj = new Bounce();
        _obj->attach(getPin(), INPUT);
    }

    void setDebounce(int value) {
        _obj->interval(value);
    }

    const bool hasValue() override {
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
    std::vector<Switch*> _items;
    OnSwitchChangeState _onChange;
};

extern Switches mySwitches;
