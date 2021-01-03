#pragma once
#include "Global.h"
#include <Arduino.h>
#include "GyverFilters.h"

class SensorUltrasonic;

typedef std::vector<SensorUltrasonic> MySensorUltrasonicVector;

class SensorUltrasonic {
public:

    SensorUltrasonic(String key, unsigned long interval, unsigned int trig, unsigned int echo, int map1, int map2, int map3, int map4, float c);
    ~SensorUltrasonic();

    void loop();
    void readUltrasonic();

private:

    unsigned long currentMillis;
    unsigned long prevMillis;
    unsigned long difference;

    unsigned long _interval;

    String _key;
    unsigned int _echo;
    unsigned int _trig;

    int _map1;
    int _map2;
    int _map3;
    int _map4;

    float _c;

};

extern MySensorUltrasonicVector* mySensorUltrasonic;

extern void ultrasonic();


