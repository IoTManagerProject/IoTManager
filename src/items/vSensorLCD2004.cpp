#include "Consts.h"
#ifdef EnableSensorLCD2004
#include "items/vSensorLCD2004.h"
#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Global.h"
#include "Utils/StringUtils.h"
#include <map>

#include <Arduino.h>

SensorLCD2004::SensorLCD2004(unsigned long interval, unsigned int pin, unsigned int index, String addr, String key) {
    _interval = interval * 1000;
    _key = key;
    _pin = pin;
    _index = index;
    _addr = addr;


}

SensorLCD2004::~SensorLCD2004() {}

void SensorLCD2004::loop() {
    currentMillis = millis();
    difference = currentMillis - prevMillis;
    if (difference >= _interval) {
        prevMillis = millis();
        readLCD2004();
    }
}

void SensorLCD2004::readLCD2004() {
    //if (_addr == "") {
    //    sensors->getAddress(deviceAddress, _index);
    //} else {
    //    string2hex(_addr.c_str(), deviceAddress);
    //}

    

    //eventGen2(_key, String(value));
    //jsonWriteStr(configLiveJson, _key, String(value));
    //publishStatus(_key, String(value));
    //char addrStr[20] = "";
    //hex2string(deviceAddress, 8, addrStr); 
    //SerialPrint("I", "Sensor", "'" + _key + "' data: " + String(value) + "' addr: " + String(addrStr));
}

MySensorLCD2004Vector* mySensorLCD20042 = nullptr;

void LCD2004() {
    myLineParsing.update();
    String interval = myLineParsing.gint();
    String pin = myLineParsing.gpin();
    String index = myLineParsing.gindex();
    String key = myLineParsing.gkey();
    String addr = myLineParsing.gaddr();
    myLineParsing.clear();

    static bool firstTime = true;
    if (firstTime) mySensorLCD20042 = new MySensorLCD2004Vector();
    firstTime = false;
    mySensorLCD20042->push_back(SensorLCD2004(interval.toInt(), pin.toInt(), index.toInt(), addr, key));
}
#endif
