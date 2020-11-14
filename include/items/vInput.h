#pragma once
#include <Arduino.h>

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

extern void input();
extern void inputExecute();
