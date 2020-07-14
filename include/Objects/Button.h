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
    Button(const String& name, const String& assign, const String& value) : Item{name, assign}, Value{VT_INT} {};

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

class VirtualButton : public Button {
   public:
    VirtualButton(const String& name, const String& assign, const String& value) : Button{name, assign, value} {};
};

class ScenButton : public Button {
   public:
    ScenButton(const String& name, const String& assign, const String& value) : Button{name, assign, value} {};

   protected:
    void onValueUpdate(const String& value) override {
        bool state = getValue().toInt();
        config.general()->enableScenario(state);
    }
};

class PinButton : public Button,
                  public PinAssigned {
   public:
    PinButton(const String& name, const String& assign, const String& value) : Button{name, assign, value},
                                                                               PinAssigned{this} {
        Serial.printf("%s onCreate: %d\n", getName(), getPin());
        pinMode(getPin(), OUTPUT);
    };

   protected:
    void onValueUpdate(const String& value) override {
        Serial.printf("%s onValueUpdate (pin:%d, inverted:%d): %li\n", getName(), getPin(), isInverted(), value.toInt());
        digitalWrite(getPin(), isInverted() ? value.toInt() : !value.toInt());
    }
};
