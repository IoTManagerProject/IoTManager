//#include "items/test.h"
//#include "BufferExecute.h"
//#include "Class/LineParsing.h"
//#include "Global.h"
//
//// Outside of class
//SensorImpulsIn *pointerToClass;  // declare a pointer to SensorImpulsIn class
//
//static void outsideInterruptHandler(void) {   // define global handler
//    pointerToClass->classInterruptHandler();  // calls class member handler
//}
//
//SensorImpulsIn::SensorImpulsIn(const paramsImpulsIn& paramsImpuls) {
//    _paramsImpuls = paramsImpulsIn(paramsImpuls);
//
//    pinMode(paramsImpuls.pin, INPUT);
//
//    pointerToClass = this;  // assign current instance to pointer (IMPORTANT!!!)
//    attachInterrupt(digitalPinToInterrupt(_paramsImpuls.pin), outsideInterruptHandler, CHANGE);
//}
//
//MySensorImpulsInVector* mySensorImpulsIn = nullptr;
//
//void SensorImpulsIn::classInterruptHandler(void) {
//    localPointerToCallback(digitalRead(_paramsImpuls.pin));
//}
//
//void impulsInSensor() {
//    myLineParsing.update();
//    String key = myLineParsing.gkey();
//    String pin = myLineParsing.gpin();
//    String c = myLineParsing.gc();
//    String k = myLineParsing.gk();
//    myLineParsing.clear();
//
//    static paramsImpulsIn paramsImpuls;
//
//    paramsImpuls.key = key;
//    paramsImpuls.pin = pin.toInt();
//    paramsImpuls.c = c.toFloat();
//    paramsImpuls.k = k.toFloat();
//
//    static bool firstTime = true;
//    if (firstTime) mySensorImpulsIn = new MySensorImpulsInVector();
//    firstTime = false;
//    mySensorImpulsIn->push_back(SensorImpulsIn(paramsImpuls));
//}