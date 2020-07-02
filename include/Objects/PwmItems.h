
#pragma once

#include "Base/Named.h"
#include "Base/Assigned.h"
#include "Base/Stateble.h"

class Pwm : public Named,
            public Assigned,
            public Stateble {
   public:
    Pwm(const String& name, const String& assigned, const String& value) : Named{name.c_str()},
                                                                           Assigned{assigned.c_str()},
                                                                           Stateble{value.toInt()} {};
    void onInit() override {
        pinMode(getPin(), OUTPUT);
    };

    void onStateChange() override {
        analogWrite(getPin(), getState());
    };
};

class Pwms {
   public:
    Pwms();
    Pwm* add(String name, String assign, String param);
    Pwm* at(size_t index);
    Pwm* get(const String name);
    size_t count();

   private:
    Pwm* last();

   private:
    std::vector<Pwm> _items;
};

extern Pwms myPwm;