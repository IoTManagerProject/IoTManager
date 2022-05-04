#include "Consts.h"
#ifdef EnableSensorBmp280
#include "items/vSensorBmp280.h"

#include <Arduino.h>

#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Global.h"

Adafruit_BMP280* bmp = nullptr;

SensorBmp280::SensorBmp280(const paramsBmp& paramsTmp, const paramsBmp& paramsPrs) {
    _paramsTmp = paramsBmp(paramsTmp);
    _paramsPrs = paramsBmp(paramsPrs);

    if (!bmp) {
        bmp = new Adafruit_BMP280;
    }

    bmp->getTemperatureSensor();
    bmp->getPressureSensor();
    bmp->begin(hexStringToUint8(_paramsPrs.addr));
}

SensorBmp280::~SensorBmp280() {}

void SensorBmp280::loop() {
    difference = millis() - prevMillis;
    if (difference >= _paramsPrs.interval) {
        prevMillis = millis();
        read();
    }
}

void SensorBmp280::read() {
    float tmp = bmp->readTemperature();
    float prs = bmp->readPressure();
    prs = prs / 1.333224 / 100;

    tmp = tmp * _paramsTmp.c;
    prs = prs * _paramsPrs.c;

    eventGen2(_paramsTmp.key, String(tmp));
    jsonWriteStr(configLiveJson, _paramsTmp.key, String(tmp));
    publishStatus(_paramsTmp.key, String(tmp));
     String path = mqttRootDevice + "/" +_paramsTmp.key + "/status";
    String json = "{}";
    jsonWriteStr(json, "status", String(tmp));
    String MyJson = json; 
    jsonWriteStr(MyJson, "topic", path); 
    #ifdef WEBSOCKET_ENABLED
    ws.textAll(MyJson);
    #endif
    SerialPrint("I", "Sensor", "'" + _paramsTmp.key + "' data: " + String(tmp));

    eventGen2(_paramsPrs.key, String(prs));
    jsonWriteStr(configLiveJson, _paramsPrs.key, String(prs));
    publishStatus(_paramsPrs.key, String(prs));
        path = mqttRootDevice + "/" +_paramsPrs.key + "/status";
     json = "{}";
    jsonWriteStr(json, "status", String(prs));
     MyJson = json; 
    jsonWriteStr(MyJson, "topic", path); 
    #ifdef WEBSOCKET_ENABLED
    ws.textAll(MyJson);
    #endif
    SerialPrint("I", "Sensor", "'" + _paramsPrs.key + "' data: " + String(prs));
}

MySensorBmp280Vector* mySensorBmp280 = nullptr;

void bmp280Sensor() {
    myLineParsing.update();
    String key = myLineParsing.gkey();
    String addr = myLineParsing.gaddr();
    String interval = myLineParsing.gint();
    String c = myLineParsing.gc();
    myLineParsing.clear();

    static int enterCnt = -1;
    enterCnt++;

    static paramsBmp paramsTmp;
    static paramsBmp paramsHum;
    static paramsBmp paramsPrs;

    if (enterCnt == 0) {
        paramsTmp.key = key;
        paramsTmp.c = c.toFloat();
    }

    if (enterCnt == 1) {
        paramsPrs.key = key;
        paramsPrs.addr = addr;
        paramsPrs.interval = interval.toInt() * 1000;
        paramsPrs.c = c.toFloat();

        static bool firstTime = true;
        if (firstTime) mySensorBmp280 = new MySensorBmp280Vector();
        firstTime = false;
        mySensorBmp280->push_back(SensorBmp280(paramsTmp, paramsPrs));
    }
}
#endif