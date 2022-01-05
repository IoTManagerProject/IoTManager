#ifdef EnableSensorLCD2004
#pragma once
#include <Arduino.h>
#include <OneWire.h>
#include "Global.h"

//ИНТЕГРИРУЮ: следим за наименованиями далее
class SensorLCD2004;

typedef std::vector<SensorLCD2004> MySensorLCD2004Vector;

class SensorLCD2004 {
   public:
   //ИНТЕГРИРУЮ: обращаем внимание на параметры, берутся из таблицы настроек
    SensorLCD2004(unsigned long interval, unsigned int pin, unsigned int index, String addr, String key);
    ~SensorLCD2004();

    void loop();
    void readLCD2004();

   private:
    unsigned long currentMillis;
    unsigned long prevMillis;
    unsigned long difference;
    unsigned long _interval;
    String _key;
    String _addr;
    unsigned int _pin;
    unsigned int _index;
};

extern MySensorLCD2004Vector* mySensorLCD20042;

extern void LCD2004();
#endif
