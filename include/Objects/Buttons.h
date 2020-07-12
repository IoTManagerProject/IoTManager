#pragma once

#include <Arduino.h>
#include <functional>

#include "Base/Item.h"
#include "Base/Assigned.h"

class ButtonItem : public Item,
                   public PinAssigned {
   public:
    ButtonItem(const String& name, const String& assign, const String& value) : Item{name, assign},
                                                                                PinAssigned{this} {};

    void setInverted(bool value) {
        _inverted = value;
    }

    bool isInverted() {
        return _inverted;
    }

    void toggleState() {
        bool state = !getValue().toInt();
        setValue(String(state));
    }

   private:
    bool _inverted;
};

class Button : public ButtonItem {
   public:
    Button(const String& name, const String& assign, const String& value) : ButtonItem{name, assign, value} {};

    void onAssign() override {
        pinMode(getPin(), OUTPUT);
    }

    void onValueUpdate(const String& value) override {
        digitalWrite(getPin(), isInverted() ? value.toInt() : !value.toInt());
    }
};

class Buttons {
   public:
    Button* add(const String& name, const String& assign, const String& value, const String& inverted);
    Button* last();
    Button* at(size_t index);
    Button* get(const String name);
    size_t count();

   private:
    std::vector<Button> _items;
};

extern Buttons myButtons;