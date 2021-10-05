#ifdef EnableInput
#pragma once
#include <Arduino.h>

#include "Consts.h"
#include "Global.h"

class Input;

typedef std::vector<Input> MyInputVector;

class Input {
   public:
    Input(String key, String widget);
    ~Input();

    void execute(String value);

   private:
    String _key;
};

extern MyInputVector* myInput;

extern void inputValue();
extern void inputExecute();
#endif
