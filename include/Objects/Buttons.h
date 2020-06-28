#pragma once

#include <Arduino.h>
#include <Bounce2.h>

struct Button_t {
    uint8_t num;
    uint8_t pin;
    int delay;
    Bounce* obj;
    Button_t(uint8_t num, uint8_t pin, int delay) : num{num}, pin{pin}, delay{delay}, obj{nullptr} {};
};

class Buttons {
   public:
    Buttons();
    Button_t* at(size_t index);
    Bounce* get(uint8_t num);
    Bounce* create(uint8_t num, uint8_t pin, int delay);
    size_t count();

   private:
    std::vector<Button_t> _items;
};

extern Buttons myButtons;