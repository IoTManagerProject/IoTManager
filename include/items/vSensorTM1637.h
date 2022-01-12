#ifdef EnableSensorTM1637
#pragma once
#include <Arduino.h>
#include <OneWire.h>
#include "Global.h"
#include "LiquidCrystal_I2C.h"


class SensorTM1637;

typedef std::vector<SensorTM1637> MySensorTM1637Vector;

class SensorTM1637 {
   public:
    SensorTM1637(String key, unsigned long interval, unsigned int x, unsigned int y, String val, String descr);
    ~SensorTM1637();

    void loop();
    void writeTM1637();
    void execute(String command);
    String _key;
    void printBlankStr(int strSize);

   private:
    unsigned long currentMillis;
    unsigned long prevMillis;
    unsigned long difference;
 
    unsigned long _interval;
    unsigned int _x;
    unsigned int _y;
    String _val;
    String _descr;
    int _prevStrSize;
};

extern MySensorTM1637Vector* mySensorTM16372;

extern void TM1637();
#endif
