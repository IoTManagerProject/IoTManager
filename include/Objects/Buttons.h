#pragma once

#include <Arduino.h>

#include "Base/Item.h"

#include <functional>

#include <Bounce2.h>

class ButtonItem : public Item {
   public:
    ButtonItem(const String& name, const String& pin, const String& value) : Item(name, pin, value){};
    virtual void onStateChange();
    virtual bool update();
};

class Button : public ButtonItem {
   public:
    Button(const String& name, const String& pin, const String& value) : ButtonItem{name, pin, value}, _obj{nullptr} {
        _obj = new Bounce();
        _obj->attach(getPin(), INPUT);
    }

    void onStateChange() override {
        digitalWrite(getPin(), getState());
    }

    bool update() {
        bool res = false;
        if (_obj) {
            res = _obj->update();
            _state = _obj->read();
        }
        return res;
    }

   private:
    Bounce* _obj;
};

typedef std::function<void(Button*, uint8_t)> OnButtonChangeState;

class Buttons {
   public:
    Button* add(String name, String assign, String value);
    void loop();
    Button* last();
    Button* at(size_t index);
    Button* get(const String name);
    size_t count();

    void setOnChangeState(OnButtonChangeState);

   private:
    std::vector<Button> _items;

    OnButtonChangeState _onChange;
};

extern Buttons myButtons;