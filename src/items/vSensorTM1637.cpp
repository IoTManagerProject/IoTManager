#include "Consts.h"
#ifdef EnableSensorTM1637
#include "items/vSensorTM1637.h"
#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Global.h"
#include "Utils/StringUtils.h"
#include <map>

#include <Arduino.h>

const uint8_t segmentsVal[] = {0x77, 0x7f, 0x39, 0x3f, 0x79, 0x71, 0x3d, 0x76, 0x1e, 0x38, 0x37, 0x3f, 0x73, 0x6d, 0x3e, 0x6e, 0x5f, 0x7c, 0x58, 0x5e, 0x7b, 0x71, 0x74, 0x10, 0x0e, 0x06, 0x54, 0x5c, 0x67, 0x50, 0x78, 0x1c, 0x6e, 0x40, 0x08, 0x48, 0x00, 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f};
char segmentsIndex[] =        {'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'J',  'L',  'N',  'O',  'P',  'S',  'U',  'Y',  'a',  'b',  'c',  'd',  'e',  'f',  'h',  'i',  'j',  'l',  'n',  'o',  'q',  'r',  't',  'u',  'y',  '-',  '_',  '=',  ' ',  '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9'};


std::map<int, DisplayObj> displayObjects;

uint8_t char2Segment(char ch) {
    for (int i=0; i<sizeof(segmentsIndex); i++) {
        if (ch == segmentsIndex[i]) return segmentsVal[i];
    }

    return 0;
}

SensorTM1637::SensorTM1637(String key, int pin1, int pin2, unsigned long interval, unsigned int c, unsigned int k, String val, String descr) {
    _key = key;
    _interval = interval * 1000;
    _c = c;
    _k = k;
    _val = val;
    _descr = descr;
    
    if (displayObjects.find(pin1) == displayObjects.end()) {
        _disp = new TM1637Display(pin1, pin2);
        DisplayObj dispObj;
        dispObj.curIndex = 0;
        dispObj.disp = _disp;
        displayObjects[pin1] = dispObj;

        _disp->setBrightness(0x0f);
        _disp->clear();
    } else {
        _disp = displayObjects[pin1].disp;
    }
}

SensorTM1637::~SensorTM1637() {}

void SensorTM1637::execute(String command) {
    if (command == "noDisplay") _disp->setBrightness(0x00, false);
    else if (command == "display") _disp->setBrightness(0x0f, true);
    else if (command == "setBrightness") {
        String par = sCmd.next();
        _disp->setBrightness(par.toInt());
    }
    else if (command == "descr") {
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
    if (_disp != nullptr) {
        if (_descr != "none") {
            uint8_t segments[] = {0};
            segments[0] = char2Segment(_descr.c_str()[0]);
            _disp->setSegments(segments, 1, 0);  //выводим поле описания в самом первой секции экрана, один символ
        }
        String tmpStr = getValue(_val);
        if (tmpStr == "no value") tmpStr = _val;
        
        _disp->showNumberDec(tmpStr.toInt(), false, _c, _k);
    }
}

MySensorTM1637Vector* mySensorTM1637 = nullptr;

void TM1637Execute() {
    String key = sCmd.order();
    String command = sCmd.next();
    
    for (unsigned int i = 0; i < mySensorTM1637->size(); i++) {
        if (mySensorTM1637->at(i)._key == key) mySensorTM1637->at(i).execute(command);
    }
}

void TM1637() {
    myLineParsing.update();
    String key = myLineParsing.gkey();
    String pins = myLineParsing.gpin();
    String interval = myLineParsing.gint();
    String c = myLineParsing.gc();
    String k = myLineParsing.gk();
    String val = myLineParsing.gval();
    String descr = myLineParsing.gdescr();
    myLineParsing.clear();
    
    int pin1 = selectFromMarkerToMarker(pins, ",", 0).toInt();
    int pin2 = selectFromMarkerToMarker(pins, ",", 1).toInt();

    static bool firstTime = true;
    if (firstTime) mySensorTM1637 = new MySensorTM1637Vector();
    firstTime = false;
    mySensorTM1637->push_back(SensorTM1637(key, pin1, pin2, interval.toInt(), c.toInt(), k.toInt(), val, descr));

    sCmd.addCommand(key.c_str(), TM1637Execute);
}

#endif
