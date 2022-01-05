#include "Consts.h"
#ifdef EnableSensorLCD2004
#include "items/vSensorLCD2004.h"
#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Global.h"
#include "Utils/StringUtils.h"
#include <map>

#include <Arduino.h>


LiquidCrystal     *LCDLPT;
LiquidCrystal_I2C *LCDI2C;

SensorLCD2004::SensorLCD2004(String key, unsigned long interval, unsigned int x, unsigned int y, String val, String descr) {
    _key = key;
    _interval = interval * 1000;
    _x = x;
    _y = y;
    _val = val;
    _descr = descr;

}

SensorLCD2004::~SensorLCD2004() {}

void SensorLCD2004::loop() {
    currentMillis = millis();
    difference = currentMillis - prevMillis;
    if (difference >= _interval) {
        prevMillis = millis();
        writeLCD2004();
    }
}

void SensorLCD2004::writeLCD2004() {
    String tmpStr;
    tmpStr = getValue(_val);
    if (tmpStr == "no value") tmpStr = _val;

    if (_descr != "none") tmpStr = _descr + " " + tmpStr;

    if(LCDI2C != nullptr) {
        LCDI2C->setCursor(_x, _y); 
        LCDI2C->print(tmpStr);    
    }

    if(LCDLPT != nullptr) {
        LCDLPT->setCursor(_x, _y); 
        LCDLPT->print(tmpStr);    
    }

    //eventGen2(_key, String(_val));
    //jsonWriteStr(configLiveJson, _key, String(_val));
    //publishStatus(_key, String(_val));
    SerialPrint("I", "Sensor", "'" + _key + "' data2: " + String(tmpStr));
}

MySensorLCD2004Vector* mySensorLCD20042 = nullptr;

void lcd2004() {
    myLineParsing.update();
    String key = myLineParsing.gkey();
    String pin = myLineParsing.gpin();
    String addr = myLineParsing.gaddr();
    String interval = myLineParsing.gint();
    String c = myLineParsing.gc();
    String k = myLineParsing.gk();
    String val = myLineParsing.gval();
    String descr = myLineParsing.gdescr();
    myLineParsing.clear();
    
    int x = selectFromMarkerToMarker(c, ",", 0).toInt();
    int y = selectFromMarkerToMarker(c, ",", 1).toInt();
    int w = selectFromMarkerToMarker(k, ",", 1).toInt();  //количество столбцов
    int h = selectFromMarkerToMarker(k, ",", 1).toInt();  //количество строк
 
    if (LCDI2C == nullptr && LCDLPT == nullptr) {  //инициализации экрана еще не было
        if (addr == "") {
            int pin1 = selectFromMarkerToMarker(pin, ",", 0).toInt();
            int pin2 = selectFromMarkerToMarker(pin, ",", 1).toInt();
            int pin3 = selectFromMarkerToMarker(pin, ",", 2).toInt();
            int pin4 = selectFromMarkerToMarker(pin, ",", 3).toInt();
            int pin5 = selectFromMarkerToMarker(pin, ",", 4).toInt();
            int pin6 = selectFromMarkerToMarker(pin, ",", 5).toInt();
            int pin7 = selectFromMarkerToMarker(pin, ",", 6).toInt();
            int pin8 = selectFromMarkerToMarker(pin, ",", 7).toInt();
            int pin9 = selectFromMarkerToMarker(pin, ",", 8).toInt();
            int pin10 = selectFromMarkerToMarker(pin, ",", 9).toInt();
            if (pin7) {
                LCDLPT = new LiquidCrystal(pin1, pin2, pin3, pin4, pin5, pin6, pin7, pin8, pin9, pin10);
            } else {
                LCDLPT = new LiquidCrystal(pin1, pin2, pin3, pin4, pin5, pin6);
            }
            if(LCDLPT != nullptr) LCDLPT->begin(w, h);
            //val = (String)pin1 + " " + (String)pin2 + " " + (String)pin3 + " " + (String)pin4 + " " + (String)pin5 + " " + (String)pin6 + " " + (String)pin7 + " " + (String)pin8 + " " + (String)pin9 + " " + (String)pin10; 
        } else {
            LCDI2C = new LiquidCrystal_I2C(hexStringToUint8(addr));
            if(LCDI2C != nullptr) {
                LCDI2C->begin(w, h);
                LCDI2C->backlight();
            }
        }
    }


    static bool firstTime = true;
    if (firstTime) mySensorLCD20042 = new MySensorLCD2004Vector();
    firstTime = false;
    mySensorLCD20042->push_back(SensorLCD2004(key, interval.toInt(), x, y, val, descr));
}
#endif
