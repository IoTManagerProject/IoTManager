#ifdef EnableSensorTM1637
#pragma once
#include <Arduino.h>
#include <OneWire.h>
#include "Global.h"
#include <TM1637Display.h>

struct DisplayObj {
    TM1637Display* disp;
    int curIndex;
};

class SensorTM1637;

typedef std::vector<SensorTM1637> MySensorTM1637Vector;

class SensorTM1637 {
   public:
    SensorTM1637(String key, int pin1, int pin2, unsigned long interval, unsigned int c, unsigned int k, String val, String descr);
    ~SensorTM1637();

    void loop();
    void writeTM1637();
    void execute(String command);
    String _key;

   private:
    unsigned long currentMillis;
    unsigned long prevMillis;
    unsigned long difference;
    
    String _descr;
    unsigned long _interval;
    unsigned int _c;
    unsigned int _k;
    String _val;

    TM1637Display* _disp;
};

extern MySensorTM1637Vector* mySensorTM1637;

extern void TM1637();
#endif
