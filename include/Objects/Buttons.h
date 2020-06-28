#pragma once

#include <Arduino.h>
#include <Bounce2.h>

struct Button_t {
    String name;
    uint8_t pin;
    uint8_t state;
    Bounce* obj;

    Button_t(String name, uint8_t pin, bool state) : name{name}, pin{pin}, state{state} {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, state);
        obj = new Bounce();
        obj->attach(pin);
    };
};

class Buttons {
   public:
    Buttons();
    Button_t* create(String name, uint8_t pin, uint8_t state);
    Button_t* at(size_t index);
    Button_t* get(const String name);
    size_t count();

   private:
    std::vector<Button_t> _items;
};

extern Buttons myButtons;