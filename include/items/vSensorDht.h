#pragma once
#include <Arduino.h>
#include <DHTesp.h>
#include "Global.h"
#include "GyverFilters.h"

extern DHTesp* dht;

class SensorDht;

typedef std::vector<SensorDht> MySensorDhtVector;

struct tmpParams {
    unsigned long currentMillis;
    unsigned long prevMillis;
    unsigned long difference;
    unsigned long interval;
    String key;
    unsigned int pin;
    int map1;
    int map2;
    int map3;
    int map4;
    float c;
};

struct humParams {
    unsigned long currentMillis;
    unsigned long prevMillis;
    unsigned long difference;
    unsigned long interval;
    String key;
    unsigned int pin;
    int map1;
    int map2;
    int map3;
    int map4;
    float c;
};

class SensorDht {
   public:
    SensorDht();
    ~SensorDht();

    void loopTmp();
    void loopHum();

    void readTmp();
    void readHum();

    void tmpInit(const tmpParams& tmpSet);
    void humInit(const humParams& humSet);

   private:
    tmpParams _tmpSet;
    humParams _humSet;
};

extern MySensorDhtVector* mySensorDht;

extern void dhtTmp();
extern void dhtHum();