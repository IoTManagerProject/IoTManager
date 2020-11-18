#pragma once
#include <Arduino.h>
#include "Global.h"

class CountDownClass;

typedef std::vector<CountDownClass> MyCountDownVector;

class CountDownClass {
public:
    CountDownClass(String key);
    ~CountDownClass();

    void loop();
    void execute(unsigned int countDownPeriod);

private:
    unsigned long _countDownPeriod = 0;
    String _key;
    bool _start = false;

};

extern MyCountDownVector* myCountDown;

extern void countDown();
extern void countDownExecute();