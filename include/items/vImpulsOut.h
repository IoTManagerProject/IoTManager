#ifdef EnableImpulsOut
#pragma once
#include <Arduino.h>
#include "Global.h"

class ImpulsOutClass;

typedef std::vector<ImpulsOutClass> MyImpulsOutVector;

class ImpulsOutClass {
   public:
    ImpulsOutClass(unsigned int impulsPin);
    ~ImpulsOutClass();

    void loop();
    void execute(unsigned long impulsPeriod, unsigned int impulsCount);

   private:
    unsigned long currentMillis;
    unsigned long prevMillis;
    unsigned long difference;
    unsigned long _impulsPeriod = 0;
    unsigned int _impulsCount = 0;
    unsigned int _impulsCountBuf = 0;
    unsigned int _impulsPin = 0;

};

extern MyImpulsOutVector* myImpulsOut;

extern void impuls();
extern void impulsExecute();
#endif
