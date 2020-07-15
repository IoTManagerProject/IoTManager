#pragma once

#include <Arduino.h>

#include <Objects/Pwm.h>

class Pwms {
   public:
    Pwms();
    Pwm* add(String name, String assign);
    Pwm* at(size_t index);
    Pwm* get(const String name);
    size_t count();

   private:
    Pwm* last();

   private:
    std::vector<Pwm*> _items;
};

extern Pwms pwms;