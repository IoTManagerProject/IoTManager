
#pragma once

#include <Arduino.h>

#include "Item.h"

class PwmItem : public Nameble,
                public Assignable,
                public Stateble {
   public:
    PwmItem(const String& name, const String& assign, const String& state) : Nameble{name.c_str()},
                                                                             Assignable{assign.c_str()},
                                                                             Stateble{state.toInt()} {
        pinMode(getPin(), OUTPUT);
    };

    void onStateChange() const override {
        analogWrite(getPin(), getState());
    };
};

class Pwms {
   public:
    Pwms();
    PwmItem* add(String name, String assign, String param);
    PwmItem* at(size_t index);
    PwmItem* get(const String name);
    size_t count();

   private:
    PwmItem* last();

   private:
    std::vector<PwmItem> _items;
};

extern Pwms myPwms;