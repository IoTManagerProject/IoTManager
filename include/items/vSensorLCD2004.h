#ifdef EnableSensorLCD2004
#pragma once
#include <Arduino.h>
#include <OneWire.h>
#include "Global.h"
#include "LiquidCrystal_I2C.h"


class SensorLCD2004;

typedef std::vector<SensorLCD2004> MySensorLCD2004Vector;

class SensorLCD2004 {
   public:
    SensorLCD2004(String key, unsigned long interval, unsigned int x, unsigned int y, String val, String descr);
    ~SensorLCD2004();

    void loop();
    void writeLCD2004();
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

extern MySensorLCD2004Vector* mySensorLCD20042;

extern void lcd2004();
#endif
