#pragma once
#include <Arduino.h>

#include "Global.h"

class ImpulsOutClass;

typedef std::vector<ImpulsOutClass> MyImpulsOutVector;

class ImpulsOutClass {
   public:
    ImpulsOutClass(unsigned long impulsPeriod, unsigned int impulsCount, unsigned int impulsPin);
    ~ImpulsOutClass();

    void loop();
    void activate();

   private:
    unsigned long currentMillis;
    unsigned long prevMillis;

    unsigned long _impulsPeriod;
    unsigned int _impulsCount;
    unsigned int _impulsCountBuf;
    unsigned int _impulsPin;

};

extern MyImpulsOutVector* myImpulsOut;
