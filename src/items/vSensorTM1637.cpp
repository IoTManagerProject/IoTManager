#include "Consts.h"
#ifdef EnableSensorTM1637
#include "items/vSensorTM1637.h"
#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Global.h"
#include "Utils/StringUtils.h"
#include <map>

#include <Arduino.h>

LiquidCrystal_I2C *LCDI2C;

SensorTM1637::SensorTM1637(String key, unsigned long interval, unsigned int x, unsigned int y, String val, String descr) {
    _key = key;
    _interval = interval * 1000;
    _x = x;
    _y = y;
    _val = val;
    _descr = descr;
    _prevStrSize = 0;
}

SensorTM1637::~SensorTM1637() {}

//печать пустой строки нужной длинны для затирания предыдущего значения на экране
void SensorTM1637::printBlankStr(int strSize){
    String tmpStr = "";
    for(int i=0; i<strSize; i++) tmpStr += " ";
    LCDI2C->setCursor(_x, _y); 
    LCDI2C->print(tmpStr);
}

void SensorTM1637::execute(String command) {
    if (command == "noBacklight") LCDI2C->noBacklight();
    else if (command == "backlight") LCDI2C->backlight();
    else if (command == "noDisplay") LCDI2C->noDisplay();
    else if (command == "display") LCDI2C->display();
    else if (command == "x") {
        printBlankStr(_prevStrSize);
        String par = sCmd.next();
        _x = par.toInt();
    }
    else if (command == "y") {
        printBlankStr(_prevStrSize);
        String par = sCmd.next();
        _y = par.toInt();
    }
    else if (command == "descr") {
        printBlankStr(_prevStrSize);
        String par = sCmd.next();
        _descr = par;
    }
    else {  //не команда, значит данные
        _val = command;
    }

    writeTM1637();
}

void SensorTM1637::loop() {
    currentMillis = millis();
    difference = currentMillis - prevMillis;
    if (difference >= _interval) {
        prevMillis = millis();
        writeTM1637();
    }
}

void SensorTM1637::writeTM1637() {
    if (LCDI2C != nullptr) {
        printBlankStr(_prevStrSize);

        String tmpStr = getValue(_val);
        if (tmpStr == "no value") tmpStr = _val;
        if (_descr != "none") tmpStr = _descr + " " + tmpStr;    
        LCDI2C->setCursor(_x, _y);
        LCDI2C->print(tmpStr); 

        _prevStrSize = tmpStr.length();
    }
}

MySensorTM1637Vector* mySensorTM16372 = nullptr;

void TM1637Execute() {
    String key = sCmd.order();
    String command = sCmd.next();
    
    for (unsigned int i = 0; i < mySensorTM16372->size(); i++) {
        if (mySensorTM16372->at(i)._key == key) mySensorTM16372->at(i).execute(command);
    }
}

void TM1637() {
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
    if (firstTime) mySensorTM16372 = new MySensorTM1637Vector();
    firstTime = false;
    mySensorTM16372->push_back(SensorTM1637(key, interval.toInt(), x, y, val, descr));

    sCmd.addCommand(key.c_str(), TM1637Execute);
}

#endif
