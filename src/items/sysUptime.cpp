#include "Class/LineParsing.h"
#include "ItemsCmd.h"
#include "Global.h"
#include <Arduino.h>

void sysUptime() {
    myLineParsing.update();
    String key = myLineParsing.gkey();
    sCmd.addCommand(key.c_str(), uptimeReading);
    sensorReadingMap += key + ",";
    myLineParsing.clear();
}

void uptimeReading() {
    String key = sCmd.order();
    eventGen(key, "");
    String uptime = timeNow->getUptime();
    uptime.replace(":", "-");
    jsonWriteStr(configLiveJson, key, uptime);
    publishStatus(key, uptime);
    Serial.println("I sensor '" + key + "' data: " + uptime);
}