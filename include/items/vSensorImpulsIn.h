//#pragma once
//#include <Arduino.h>
//
//#include "Global.h"
//#include "PZEMSensor.h"
//#include "SoftUART.h"
//
//class SensorImpulsIn;
//
//typedef std::vector<SensorImpulsIn> MySensorImpulsInVector;
//
//struct paramsImpulsIn {
//    String key;
//    unsigned int pin;
//    float c;
//    float k;
//};
//
//class SensorImpulsIn {
//   public:
//    SensorImpulsIn(const paramsImpulsIn& paramsImpuls);
//    ~SensorImpulsIn();
//
//    void interruptHandler(void);
//
//   private:
//    paramsImpulsIn _paramsImpuls;
//};
//
//extern MySensorImpulsInVector* mySensorImpulsIn;
//
//extern void impulsInSensor();