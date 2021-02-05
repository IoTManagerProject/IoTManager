#ifdef EnableSensorNode
#pragma once
#include <Arduino.h>

#include "Global.h"

class SensorNode;

typedef std::vector<SensorNode> MySensorNodeVector;

struct paramsSensorNode {
    String tm1;
    String tm2;
    String key;
    float c;
    float k;
};

class SensorNode {
   public:
    SensorNode(const paramsSensorNode& params);
    ~SensorNode();

    void loop();
    void onChange(String newValue, String incommingKey);
    void publish();

   private:
    paramsSensorNode _params;
    long _minutesPassed;
    String _updateTime;

    unsigned long prevMillis = 0;
    unsigned long difference;
};

extern MySensorNodeVector* mySensorNode;

extern void nodeSensor();
extern void publishTimes();
#endif