#pragma once
#include <Arduino.h>

#include "Global.h"

class ButtonOut;

typedef std::vector<ButtonOut> MyButtonOutVector;

class ButtonOut {
   public:

    ButtonOut(unsigned int pin, boolean inv, String key);

    ~ButtonOut();

    void execute(String state);

   private:

    unsigned int _pin;
    boolean _inv;
    String _key;

};

extern MyButtonOutVector* myButtonOut;

extern void buttonOut();
extern void buttonOutExecute();
