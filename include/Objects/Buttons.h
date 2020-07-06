#pragma once

#include <Arduino.h>

#include "Base/Item.h"

#include <Bounce2.h>
#include <functional>

class Button : public Item {
   public:
    Button(const String& name, const String& pin, const String& value) : Item{name, pin, value} {
        pinMode(getPin(), OUTPUT);
    }
    void onStateChange() override {
        digitalWrite(getPin(), _state);
    }
};

class Buttons {
   public:
    Button* add(String name, String assign, String value);
    Button* last();
    Button* at(size_t index);
    Button* get(const String name);
    size_t count();

   private:
    std::vector<Button> _items;
};

extern Buttons myButtons;