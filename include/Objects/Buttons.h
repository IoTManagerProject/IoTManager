#pragma once

#include <Arduino.h>

#include <functional>

#include <Bounce2.h>
struct Button_t {
    String name;
    uint8_t pin;
    uint8_t state;
    Bounce* obj;

    Button_t(String name, uint8_t pin, bool state) : name{name}, pin{pin}, state{state} {
        obj = new Bounce();
        obj->attach(pin, OUTPUT);
        digitalWrite(pin, state);
    };

    bool update() {
        bool res = obj->update();
        state = obj->read();
        return res;
    }
};

typedef std::function<void(Button_t*,uint8_t)> OnButtonChangeState;

class Buttons {
   public:
    Buttons();
    Button_t* create(String name, uint8_t pin, uint8_t state);
    Button_t* at(size_t index);
    Button_t* get(const String name);
    Bounce* obj(size_t index);
    size_t count();
    void loop();
    void setOnChangeState(OnButtonChangeState);

   private:
    OnButtonChangeState onChangeState;
    std::vector<Button_t> _items;
};

extern Buttons myButtons;