#include "items/vSensorCcs811.h"

#include <Arduino.h>

#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Global.h"

SensorCcs811::SensorCcs811(const paramsCcs811& paramsCo2, const paramsCcs811& paramsPpm) {
    _paramsCo2 = paramsCcs811(paramsCo2);
    _paramsPpm = paramsCcs811(paramsPpm);

    ccs811 = new Adafruit_CCS811();

    if (!ccs811->begin()) SerialPrint("E", "Sensor CCS", "Wire not connected");
}

SensorCcs811::~SensorCcs811() {}

void SensorCcs811::loop() {
    difference = millis() - prevMillis;
    if (difference >= _paramsPpm.interval) {
        prevMillis = millis();
        read();
    }
}

void SensorCcs811::read() {
    float co2;
    float ppm;
    if (ccs811->available()) {
        if (!ccs811->readData()) {
            co2 = ccs811->geteCO2();
            ppm = ccs811->getTVOC();

            co2 = co2 * _paramsCo2.c;
            ppm = ppm * _paramsPpm.c;

            eventGen2(_paramsCo2.key, String(co2));
            jsonWriteStr(configLiveJson, _paramsCo2.key, String(co2));
            publishStatus(_paramsCo2.key, String(co2));
            SerialPrint("I", "Sensor", "'" + _paramsCo2.key + "' data: " + String(co2));

            eventGen2(_paramsPpm.key, String(ppm));
            jsonWriteStr(configLiveJson, _paramsPpm.key, String(ppm));
            publishStatus(_paramsPpm.key, String(ppm));
            SerialPrint("I", "Sensor", "'" + _paramsPpm.key + "' data: " + String(ppm));
        } else {
            SerialPrint("E", "Sensor CCS", "Error");
        }
    }
}

MySensorCcs811Vector* mySensorCcs811 = nullptr;

void ccs811Sensor() {
    myLineParsing.update();
    String key = myLineParsing.gkey();
    String addr = myLineParsing.gaddr();
    String interval = myLineParsing.gint();
    String c = myLineParsing.gc();
    myLineParsing.clear();

    static int enterCnt = -1;
    enterCnt++;

    static paramsCcs811 paramsCo2;
    static paramsCcs811 paramsPpm;

    if (enterCnt == 0) {
        paramsCo2.key = key;
        paramsCo2.interval = interval.toInt() * 1000;
        paramsCo2.c = c.toFloat();
    }

    if (enterCnt == 1) {
        paramsPpm.key = key;
        paramsPpm.addr = addr;
        paramsPpm.interval = interval.toInt() * 1000;
        paramsPpm.c = c.toFloat();

        static bool firstTime = true;
        if (firstTime) mySensorCcs811 = new MySensorCcs811Vector();
        firstTime = false;
        mySensorCcs811->push_back(SensorCcs811(paramsCo2, paramsPpm));

        enterCnt = -1;
    }
}
