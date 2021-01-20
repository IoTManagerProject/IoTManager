//#include "Arduino.h"
//#include "Global.h"
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
//    //~SensorImpulsIn();
//
//    //void begin(int interruptPin);
//    void classInterruptHandler(void);
//
//    void setCallback(void (*userDefinedCallback)(const int)) {
//        localPointerToCallback = userDefinedCallback;
//    }
//
//   private:
//    void (*localPointerToCallback)(const int);
//    paramsImpulsIn _paramsImpuls;
//};
//
//extern MySensorImpulsInVector* mySensorImpulsIn;
//
//extern void impulsInSensor();