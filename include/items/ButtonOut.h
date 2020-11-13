#pragma once
#include <Arduino.h>

#include "Global.h"

class ButtonOut;

typedef std::vector<ButtonOut> MyButtonOutVector;

class ButtonOut {
   public:

    ButtonOut(unsigned int pin, boolean inv, String key);

    ~ButtonOut();

    void init();
    void execute(String state);

   private:

    unsigned int _pin;
    boolean _inv;
    String _key;

    void addNewDelOldData(const String filename, size_t maxPoints, String payload);
};

extern MyButtonOutVector* myButtonOut;

extern void buttonOut();
extern void buttonOutExecute();
