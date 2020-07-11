#pragma once

#include <Arduino.h>
#include <functional>

#include "Base/Item.h"

class ButtonItem : public Item,
                   public Assigned {
   public:
    ButtonItem(const String& name, const String& assign, const String& value) : Item{name, assign, value},
                                                                                Assigned(assign.c_str()){};
};

class Button : public ButtonItem, public PinAssigned {
   public:
    Button(const String& name, const String& assign, const String& value, const String& inverted) : ButtonItem{name, assign, value},
                                                                                                    PinAssigned(assign.c_str()) {
        _inverted = inverted.toInt();
    };

    void onAssign() override {
        pinMode(getPin(), OUTPUT);
    }

    void onStateChange() override {
        digitalWrite(getPin(), _inverted ? _state : !_state);
    }

   private:
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