//#include "items/vSensorImpulsIn.h"
//
//#include "BufferExecute.h"
//#include "Class/LineParsing.h"
//#include "Global.h"
//#include "SoftUART.h"
//
//SensorImpulsIn::SensorImpulsIn(const paramsImpulsIn& paramsImpuls) {
//    _paramsImpuls = paramsImpulsIn(paramsImpuls);
//    pinMode(14, INPUT);
//    //pinMode(_paramsImpuls.pin, INPUT_PULLUP);
//    //attachInterrupt(digitalPinToInterrupt(14), MYinterrupt, CHANGE);
//}
//
//SensorImpulsIn::~SensorImpulsIn() {}
//
////void SensorImpulsIn::read() {
//    float voltage;  //= (impulsIn->values()->voltage * _paramsV.c) + _paramsV.k;
//
//    eventGen2(_paramsImpuls.key, String(voltage));
//    jsonWriteStr(configLiveJson, _paramsImpuls.key, String(voltage));
//    publishStatus(_paramsImpuls.key, String(voltage));
//    SerialPrint("I", "Sensor", "'" + _paramsImpuls.key + "' data: " + String(voltage));
//}
//
//MySensorImpulsInVector* mySensorImpulsIn = nullptr;
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
//
//void MYinterrupt() {
//    Serial.println("interrupt!");
//}
