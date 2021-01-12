#include "items/vSensorPzem.h"

#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Global.h"
#include "SoftUART.h"

PZEMSensor* pzem = nullptr;

SensorPzem::SensorPzem(const paramsPzem& paramsV, const paramsPzem& paramsA, const paramsPzem& paramsWatt, const paramsPzem& paramsWattHrs, const paramsPzem& paramsHz) {
    _paramsV = paramsPzem(paramsV);
    _paramsA = paramsPzem(paramsA);
    _paramsWatt = paramsPzem(paramsWatt);
    _paramsWattHrs = paramsPzem(paramsWattHrs);
    _paramsHz = paramsPzem(paramsHz);

    if (myUART) {
        if (!pzem) {
            pzem = new PZEMSensor(myUART, 0xF8);
        }
    }
}

SensorPzem::~SensorPzem() {}

void SensorPzem::loop() {
    difference = millis() - prevMillis;
    if (difference >= _paramsHz.interval) {
        prevMillis = millis();
        read();
    }
}

void SensorPzem::read() {
    if (myUART) {
        if (pzem) {
            float voltage = pzem->values()->voltage;
            float current = pzem->values()->current;
            float power = pzem->values()->power;
            float energy = pzem->values()->energy;
            float freq = pzem->values()->freq;

            eventGen2(_paramsV.key, String(voltage));
            jsonWriteStr(configLiveJson, _paramsV.key, String(voltage));
            publishStatus(_paramsV.key, String(voltage));
            SerialPrint("I", "Sensor", "'" + _paramsV.key + "' data: " + String(voltage));

            eventGen2(_paramsA.key, String(current));
            jsonWriteStr(configLiveJson, _paramsA.key, String(current));
            publishStatus(_paramsA.key, String(current));
            SerialPrint("I", "Sensor", "'" + _paramsA.key + "' data: " + String(current));

            eventGen2(_paramsWatt.key, String(power));
            jsonWriteStr(configLiveJson, _paramsWatt.key, String(power));
            publishStatus(_paramsWatt.key, String(power));
            SerialPrint("I", "Sensor", "'" + _paramsWatt.key + "' data: " + String(power));

            eventGen2(_paramsWattHrs.key, String(energy));
            jsonWriteStr(configLiveJson, _paramsWattHrs.key, String(energy));
            publishStatus(_paramsWattHrs.key, String(energy));
            SerialPrint("I", "Sensor", "'" + _paramsWattHrs.key + "' data: " + String(energy));

            eventGen2(_paramsHz.key, String(freq));
            jsonWriteStr(configLiveJson, _paramsHz.key, String(freq));
            publishStatus(_paramsHz.key, String(freq));
            SerialPrint("I", "Sensor", "'" + _paramsHz.key + "' data: " + String(freq));
        } else {
            SerialPrint("E", "Sensor PZEM", "Error, PZEM object not created");
        }
    } else {
        SerialPrint("E", "Sensor PZEM", "Error, UART switched off");
    }
}

MySensorPzemVector* mySensorPzem = nullptr;

void pzemSensor() {
    if (myUART) {
        myLineParsing.update();
        String key = myLineParsing.gkey();
        String addr = myLineParsing.gaddr();
        String interval = myLineParsing.gint();
        String c = myLineParsing.gc();
        myLineParsing.clear();

        static int enterCnt = -1;
        enterCnt++;

        static paramsPzem paramsV;
        static paramsPzem paramsA;
        static paramsPzem paramsWatt;
        static paramsPzem paramsWattHrs;
        static paramsPzem paramsHz;

        if (enterCnt == 0) {
            paramsV.key = key;
            paramsV.c = c.toFloat();
        }

        if (enterCnt == 1) {
            paramsA.key = key;
            paramsA.c = c.toFloat();
        }

        if (enterCnt == 2) {
            paramsWatt.key = key;
            paramsWatt.c = c.toFloat();
        }

        if (enterCnt == 3) {
            paramsWattHrs.key = key;
            paramsWattHrs.c = c.toFloat();
        }

        if (enterCnt == 4) {
            paramsHz.key = key;
            paramsHz.c = c.toFloat();
            paramsHz.addr = addr;
            paramsHz.interval = interval.toInt() * 1000;
            paramsHz.c = c.toFloat();

            static bool firstTime = true;
            if (firstTime) mySensorPzem = new MySensorPzemVector();
            firstTime = false;
            mySensorPzem->push_back(SensorPzem(paramsV, paramsA, paramsWatt, paramsWattHrs, paramsHz));

            enterCnt = -1;
        }
    } else {
        SerialPrint("E", "Sensor PZEM", "Error, UART switched off");
    }
}
