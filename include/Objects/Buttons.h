#pragma once

#include <Arduino.h>
#include <functional>

#include "Config.h"
#include "Base/Item.h"
#include "Base/Assigned.h"

enum ButtonType_t {
    BUTTON_VIRTUAL,
    BUTTON_GPIO,
    BUTTON_SCEN,
    BUTTON_SCEN_LINE
};

class Button : public Item,
               public Value {
   public:
    Button(const String& name, const String& assign, const String& value) : Item{name}, Value{VT_INT} {};

    virtual void onAssign() override {}

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

class ScenButton : public Button {
   public:
    ScenButton(const String& name, const String& assign, const String& value) : Button{name, assign, value} {};

    void onValueUpdate(const String& value) override {
        bool state = getValue().toInt();
        config.general()->enableScenario(state);
    }
};

class PinButton : public Button,
                  public PinAssigned {
   public:
    PinButton(const String& name, const String& assign, const String& value) : Button{name, assign, value},
                                                                               PinAssigned{this} {};

    void onAssign() override {
        pinMode(getPin(), OUTPUT);
    }

    void onValueUpdate(const String& value) override {
        digitalWrite(getPin(), isInverted() ? value.toInt() : !value.toInt());
    }
};

class Buttons {
   public:
    Button* add(const ButtonType_t type, const String& name, const String& assign, const String& value, const String& inverted);
    Button* last();
    Button* at(size_t index);
    Button* get(const String name);
    size_t count();

   private:
    std::vector<Button*> _list;
};

extern Buttons buttons;