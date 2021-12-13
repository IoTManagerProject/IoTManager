#include "Consts.h"
#ifdef EnableSensorCcs811
#include "items/vSensorCcs811.h"

#include <Arduino.h>

#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Global.h"

SensorCcs811::SensorCcs811(const paramsCcs811& paramsPpm, const paramsCcs811& paramsPpb) {
    _paramsPpm = paramsCcs811(paramsPpm);
    _paramsPpb = paramsCcs811(paramsPpb);

    ccs811 = new Adafruit_CCS811();

    if (!ccs811->begin(hexStringToUint8(_paramsPpb.addr))) SerialPrint("E", "Sensor CCS", "Wire not connected");
}

SensorCcs811::~SensorCcs811() {}

void SensorCcs811::loop() {
    difference = millis() - prevMillis;
    if (difference >= _paramsPpb.interval) {
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

            co2 = co2 * _paramsPpm.c;
            ppm = ppm * _paramsPpb.c;

            eventGen2(_paramsPpm.key, String(co2));
            jsonWriteStr(configLiveJson, _paramsPpm.key, String(co2));
            publishStatus(_paramsPpm.key, String(co2));
               String path = mqttRootDevice + "/" +_paramsPpm.key + "/status";
    String json = "{}";
    jsonWriteStr(json, "status", String(co2));
    String MyJson = json; 
    jsonWriteStr(MyJson, "topic", path); 
    ws.textAll(MyJson);
            SerialPrint("I", "Sensor", "'" + _paramsPpm.key + "' data: " + String(co2));

            eventGen2(_paramsPpb.key, String(ppm));
            jsonWriteStr(configLiveJson, _paramsPpb.key, String(ppm));
            publishStatus(_paramsPpb.key, String(ppm));
                 path = mqttRootDevice + "/" +_paramsPpb.key + "/status";
     json = "{}";
    jsonWriteStr(json, "status", String(ppm));
     MyJson = json; 
    jsonWriteStr(MyJson, "topic", path); 
    ws.textAll(MyJson);
            SerialPrint("I", "Sensor", "'" + _paramsPpb.key + "' data: " + String(ppm));
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

    static paramsCcs811 paramsPpm;
    static paramsCcs811 paramsPpb;

    if (enterCnt == 0) {
        paramsPpm.key = key;
        paramsPpm.interval = interval.toInt() * 1000;
        paramsPpm.c = c.toFloat();
    }

    if (enterCnt == 1) {
        paramsPpb.key = key;
        paramsPpb.addr = addr;
        paramsPpb.interval = interval.toInt() * 1000;
        paramsPpb.c = c.toFloat();

        static bool firstTime = true;
        if (firstTime) mySensorCcs811 = new MySensorCcs811Vector();
        firstTime = false;
        mySensorCcs811->push_back(SensorCcs811(paramsPpm, paramsPpb));

        enterCnt = -1;
    }
}
#endif