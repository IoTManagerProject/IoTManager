#pragma once

#include "Objects/Button.h"

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