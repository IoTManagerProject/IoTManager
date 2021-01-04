#include "items/vSensorUptime.h"

#include <Arduino.h>

#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "Global.h"

SensorUptime::SensorUptime(const paramsUptime& paramsUpt) {
    _paramsUpt = paramsUptime(paramsUpt);
}

SensorUptime::~SensorUptime() {}

void SensorUptime::loop() {
    difference = millis() - prevMillis;
    if (difference >= _paramsUpt.interval) {
        prevMillis = millis();
        read();
    }
}

void SensorUptime::read() {
    String upt = timeNow->getUptime();

    eventGen2(_paramsUpt.key, upt);
    jsonWriteStr(configLiveJson, _paramsUpt.key, upt);
    publishStatus(_paramsUpt.key, upt);
    SerialPrint("I", "Sensor", "'" + _paramsUpt.key + "' data: " + upt);
}

MySensorUptimeVector* mySensorUptime = nullptr;

void uptimeSensor() {
    myLineParsing.update();
    String key = myLineParsing.gkey();
    String interval = myLineParsing.gint();
    myLineParsing.clear();

    static paramsUptime paramsUpt;

    paramsUpt.key = key;
    paramsUpt.interval = interval.toInt() * 1000;

    static bool firstTime = true;
    if (firstTime) mySensorUptime = new MySensorUptimeVector();
    firstTime = false;
    mySensorUptime->push_back(SensorUptime(paramsUpt));
}
