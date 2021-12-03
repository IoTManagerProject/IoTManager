#include "Consts.h"
#ifdef EnableSensorSht20
#include "items/vSensorSht20.h"

#include <Arduino.h>

#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Global.h"

#include "Wire.h"
#include "SHT2x.h"
SHT2x* sht = nullptr;

SensorSht20::SensorSht20(const paramsSht& paramsTmp, const paramsSht& paramsHum) {
    _paramsTmp = paramsSht(paramsTmp);
    _paramsHum = paramsSht(paramsHum);

    if (!sht) {
        sht = new SHT2x;
    }

    sht->begin();

    uint8_t stat = sht->getStatus();
    Serial.print(stat, HEX);
    Serial.println();
}

SensorSht20::~SensorSht20() {}

void SensorSht20::loop() {
    difference = millis() - prevMillis;
    if (difference >= _paramsHum.interval) {
        prevMillis = millis();
        read();
    }
}

void SensorSht20::read() {
    sht->read();

    float tmp = sht->getTemperature();
    float hum = sht->getHumidity();

    tmp = tmp * _paramsTmp.c;
    hum = hum * _paramsHum.c;

    eventGen2(_paramsTmp.key, String(tmp));
    jsonWriteStr(configLiveJson, _paramsTmp.key, String(tmp));
    publishStatus(_paramsTmp.key, String(tmp));
    SerialPrint("I", "Sensor", "'" + _paramsTmp.key + "' data: " + String(tmp));

    eventGen2(_paramsHum.key, String(hum));
    jsonWriteStr(configLiveJson, _paramsHum.key, String(hum));
    publishStatus(_paramsHum.key, String(hum));
    SerialPrint("I", "Sensor", "'" + _paramsHum.key + "' data: " + String(hum));
}

MySensorSht20Vector* mySensorSht20 = nullptr;

void sht20Sensor() {
    myLineParsing.update();
    String key = myLineParsing.gkey();
    String interval = myLineParsing.gint();
    String c = myLineParsing.gc();
    myLineParsing.clear();

    static int enterCnt = -1;
    enterCnt++;

    static paramsSht paramsTmp;
    static paramsSht paramsHum;

    if (enterCnt == 0) {
        paramsTmp.key = key;
        paramsTmp.c = c.toFloat();
    }

    if (enterCnt == 1) {
        paramsHum.key = key;
        paramsHum.c = c.toFloat();
        paramsHum.interval = interval.toInt() * 1000;

        static bool firstTime = true;
        if (firstTime) mySensorSht20 = new MySensorSht20Vector();
        firstTime = false;
        mySensorSht20->push_back(SensorSht20(paramsTmp, paramsHum));
    }
}
#endif