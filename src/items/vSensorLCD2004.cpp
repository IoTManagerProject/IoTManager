#include "Consts.h"
#ifdef EnableSensorLCD2004
#include "items/vSensorLCD2004.h"
#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Global.h"
#include "Utils/StringUtils.h"
#include <map>

#include <Arduino.h>

LiquidCrystal_I2C *LCDI2C;

SensorLCD2004::SensorLCD2004(String key, unsigned long interval, unsigned int x, unsigned int y, String val, String descr) {
    _key = key;
    _interval = interval * 1000;
    _x = x;
    _y = y;
    _val = val;
    _descr = descr;
    _prevStrSize = 0;
}

SensorLCD2004::~SensorLCD2004() {}

void SensorLCD2004::execute(String command) {
    //if (command == "back") 
    
    String par = sCmd.next();
    //SerialPrint("I", "execute", command + " " + par);
}

void SensorLCD2004::loop() {
    currentMillis = millis();
    difference = currentMillis - prevMillis;
    if (difference >= _interval) {
        prevMillis = millis();
        writeLCD2004();
    }
}

void SensorLCD2004::writeLCD2004() {
    if (LCDI2C != nullptr) {
        String tmpStr = "";
        for(int i=0; i<_prevStrSize; i++) tmpStr += " ";
        LCDI2C->setCursor(_x, _y); 
        LCDI2C->print(tmpStr); 

        tmpStr = getValue(_val);
        if (tmpStr == "no value") tmpStr = _val;
        if (_descr != "none") tmpStr = _descr + " " + tmpStr;    
        LCDI2C->setCursor(_x, _y);
        LCDI2C->print(tmpStr); 

        _prevStrSize = tmpStr.length();
    }
}

MySensorLCD2004Vector* mySensorLCD20042 = nullptr;

void lcd2004Execute() {
    String key = sCmd.order();
    String command = sCmd.next();
    
    for (unsigned int i = 0; i < mySensorLCD20042->size(); i++) {
        if (mySensorLCD20042->at(i)._key == key) mySensorLCD20042->at(i).execute(command);
    }
}

void lcd2004() {
    myLineParsing.update();
    String key = myLineParsing.gkey();
    String addr = myLineParsing.gaddr();
    String interval = myLineParsing.gint();
    String c = myLineParsing.gc();
    String k = myLineParsing.gk();
    String val = myLineParsing.gval();
    String descr = myLineParsing.gdescr();
    myLineParsing.clear();
    
    int x = selectFromMarkerToMarker(c, ",", 0).toInt();
    int y = selectFromMarkerToMarker(c, ",", 1).toInt();
    int w = selectFromMarkerToMarker(k, ",", 0).toInt();  //количество столбцов
    int h = selectFromMarkerToMarker(k, ",", 1).toInt();  //количество строк
 
    if (LCDI2C == nullptr) {  //инициализации экрана еще не было
        LCDI2C = new LiquidCrystal_I2C(hexStringToUint8(addr), w, h);//hexStringToUint8(addr), w, h);
        if(LCDI2C != nullptr) {
            LCDI2C->init();
            LCDI2C->backlight();
        }    
    }


    static bool firstTime = true;
    if (firstTime) mySensorLCD20042 = new MySensorLCD2004Vector();
    firstTime = false;
    mySensorLCD20042->push_back(SensorLCD2004(key, interval.toInt(), x, y, val, descr));

    sCmd.addCommand(key.c_str(), lcd2004Execute);
}

#endif
