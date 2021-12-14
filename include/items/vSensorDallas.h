#ifdef EnableSensorDallas
#pragma once
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Global.h"

//ИНТЕГРИРУЮ: Объявляем глагольные переменные необходимые интегрируемой библиотеке
extern DallasTemperature sensors;
extern OneWire* oneWire;

//ИНТЕГРИРУЮ: следим за наименованиями далее
class SensorDallas;

typedef std::vector<SensorDallas> MySensorDallasVector;

class SensorDallas {
   public:
   //ИНТЕГРИРУЮ: обращаем внимание на параметры, берутся из таблицы настроек
    SensorDallas(unsigned long interval, unsigned int pin, unsigned int index, String addr, String key);
    ~SensorDallas();

    void loop();
    void readDallas();

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

extern MySensorDallasVector* mySensorDallas2;

extern void dallas();
#endif
