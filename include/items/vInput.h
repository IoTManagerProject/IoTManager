#pragma once
#include <Arduino.h>

#include "Global.h"

class Input;

typedef std::vector<Input> MyInputVector;

class Input {
   public:

    Input(String key);

    ~Input();

    void execute(String state);

   private:

    String _key;

    void addNewDelOldData(const String filename, size_t maxPoints, String payload);
};

extern MyInputVector* myInput;

extern void input();
extern void inputExecute();
