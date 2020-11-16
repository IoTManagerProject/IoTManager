#pragma once
#include <Arduino.h>

#include "Global.h"

class PwmOut;

typedef std::vector<PwmOut> MyPwmOutVector;

class PwmOut {
   public:

    PwmOut(unsigned int pin, String key);

    ~PwmOut();

    void execute(String state);

   private:

    unsigned int _pin;
    String _key;

};

extern MyPwmOutVector* myPwmOut;

extern void pwmOut();
extern void pwmOutExecute();
