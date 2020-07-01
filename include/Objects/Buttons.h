#pragma once

#include <Arduino.h>

#include <functional>

#include <Bounce2.h>

enum ButtonType_t { BUTTON_PINNED,
                    BUTTON_VIRTUAL,
                    BUTTON_SWITCH };

struct Button_t {
    ButtonType_t type;
    bool state;
    String name;
    String assign;
    String param;
    Bounce* obj;

    uint8_t getPin() {
        return assign.toInt();
    }

    bool getState() {
        return param.toInt();
    }

    Button_t(ButtonType_t type, String name, String assign, String param)
        : type{type}, name{name}, assign{assign}, param{param} {
        switch (type) {
            case BUTTON_SWITCH:
                obj = new Bounce();
                obj->attach(getPin(), INPUT);
                break;
            case BUTTON_PINNED:
                digitalWrite(getPin(), getState());
                obj = new Bounce();
                obj->attach(getPin(), OUTPUT);
                break;
            case BUTTON_VIRTUAL:
                obj = nullptr;
            default:
                break;
        };
    }

    bool set(bool value) {
        state = value;
        digitalWrite(getPin(), state);
        return state;
    }

    bool toggle() {
        state = !state;
        digitalWrite(getPin(), state);
        return state;
    }

    bool update() {
        bool res = false;
        if (obj) {
            res = obj->update();
            state = obj->read();
        }
        return res;
    }
};

typedef std::function<void(Button_t*, uint8_t)> OnButtonChangeState;

class Buttons {
   public:
    Buttons();
    Button_t* addSwitch(String name, String assign, String param);
    Button_t* addButton(String name, String assign, String param);

    Button_t* last();
    Button_t* at(size_t index);
    Button_t* get(const String name);
    size_t count();
    void loop();
    void setOnChangeState(OnButtonChangeState);

   private:
    OnButtonChangeState onChangeState;
    std::vector<Button_t> _items;
};

extern Buttons myButtons;