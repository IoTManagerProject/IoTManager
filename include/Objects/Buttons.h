#pragma once

#include <Arduino.h>
#include <functional>

#include "Base/Item.h"

class ButtonItem : public Item {
   public:
    ButtonItem(const String& name, const String& pin, const String& value) : Item{name, pin, value} {
        pinMode(getPin(), OUTPUT);
    }
};

class Button : public ButtonItem {
   public:
    Button(const String& name, const String& pin, const String& value, bool inverted) : ButtonItem{name, pin, value}, _inverted{inverted} {};
    void onStateChange() override {
        digitalWrite(getPin(), _inverted ? _state : !_state);
    }

    bool _inverted;
};

class Buttons {
   public:
    ButtonItem* add(const String& name, const String& assign, const String& value, const String& inverted);
    ButtonItem* last();
    ButtonItem* at(size_t index);
    ButtonItem* get(const String name);
    size_t count();

   private:
    std::vector<ButtonItem*> _items;
};

extern Buttons myButtons;