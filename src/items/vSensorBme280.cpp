#include "Consts.h"
#ifdef EnableSensorBme280
#include "items/vSensorBme280.h"

#include <Arduino.h>

#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Global.h"

Adafruit_BME280* bme = nullptr;

SensorBme280::SensorBme280(const paramsBme& paramsTmp, const paramsBme& paramsHum, const paramsBme& paramsPrs) {
    _paramsTmp = paramsBme(paramsTmp);
    _paramsHum = paramsBme(paramsHum);
    _paramsPrs = paramsBme(paramsPrs);

    if (!bme) {
        bme = new Adafruit_BME280;
    }

    bme->getTemperatureSensor();
    bme->getPressureSensor();
    bme->getHumiditySensor();
    bme->begin(hexStringToUint8(_paramsPrs.addr));
}

SensorBme280::~SensorBme280() {}

void SensorBme280::loop() {
    difference = millis() - prevMillis;
    if (difference >= _paramsPrs.interval) {
        prevMillis = millis();
        read();
    }
}

void SensorBme280::read() {
    float tmp = bme->readTemperature();
    float hum = bme->readHumidity();
    float prs = bme->readPressure();
    prs = prs / 1.333224 / 100;

    tmp = tmp * _paramsTmp.c;
    hum = hum * _paramsHum.c;
    prs = prs * _paramsPrs.c;

    eventGen2(_paramsTmp.key, String(tmp));
    jsonWriteStr(configLiveJson, _paramsTmp.key, String(tmp));
    publishStatus(_paramsTmp.key, String(tmp));
      String path = mqttRootDevice + "/" +_paramsTmp.key + "/status";
    String json = "{}";
    jsonWriteStr(json, "status", String(tmp));
    String MyJson = json; 
    jsonWriteStr(MyJson, "topic", path); 
    ws.textAll(MyJson);
    SerialPrint("I", "Sensor", "'" + _paramsTmp.key + "' data: " + String(tmp));

    eventGen2(_paramsHum.key, String(hum));
    jsonWriteStr(configLiveJson, _paramsHum.key, String(hum));
    publishStatus(_paramsHum.key, String(hum));
      path = mqttRootDevice + "/" +_paramsHum.key + "/status";
     json = "{}";
    jsonWriteStr(json, "status", String(hum));
     MyJson = json; 
    jsonWriteStr(MyJson, "topic", path); 
    ws.textAll(MyJson);
    SerialPrint("I", "Sensor", "'" + _paramsHum.key + "' data: " + String(hum));

    eventGen2(_paramsPrs.key, String(prs));
    jsonWriteStr(configLiveJson, _paramsPrs.key, String(prs));
    publishStatus(_paramsPrs.key, String(prs));
      path = mqttRootDevice + "/" +_paramsPrs.key + "/status";
     json = "{}";
    jsonWriteStr(json, "status", String(prs));
     MyJson = json; 
    jsonWriteStr(MyJson, "topic", path); 
    ws.textAll(MyJson);
    SerialPrint("I", "Sensor", "'" + _paramsPrs.key + "' data: " + String(prs));
}

MySensorBme280Vector* mySensorBme280 = nullptr;

void bme280Sensor() {
    myLineParsing.update();
    String key = myLineParsing.gkey();
    String addr = myLineParsing.gaddr();
    String interval = myLineParsing.gint();
    String c = myLineParsing.gc();
    myLineParsing.clear();

    static int enterCnt = -1;
    enterCnt++;

    static paramsBme paramsTmp;
    static paramsBme paramsHum;
    static paramsBme paramsPrs;

    if (enterCnt == 0) {
        paramsTmp.key = key;
        paramsTmp.c = c.toFloat();
    }

    if (enterCnt == 1) {
        paramsHum.key = key;
        paramsHum.c = c.toFloat();
    }

    if (enterCnt == 2) {
        paramsPrs.key = key;
        paramsPrs.addr = addr;
        paramsPrs.interval = interval.toInt() * 1000;
        paramsPrs.c = c.toFloat();

        static bool firstTime = true;
        if (firstTime) mySensorBme280 = new MySensorBme280Vector();
        firstTime = false;
        mySensorBme280->push_back(SensorBme280(paramsTmp, paramsHum, paramsPrs));
    }
}
#endif