#include "Class/LineParsing.h"
#include "BufferExecute.h"
#include "Global.h"
#include <Arduino.h>

void sysUptime() {
    myLineParsing.update();
    String key = myLineParsing.gkey();
    sCmd.addCommand(key.c_str(), uptimeReading);
    sensorReadingMap30sec += key + ",";
    myLineParsing.clear();
}

void uptimeReading() {
    String key = sCmd.order();
    String uptime = timeNow->getUptime();
    jsonWriteStr(configLiveJson, key, uptime);
    publishStatus(key, uptime);
    SerialPrint("I", "Sensor", "'" + key + "' data: " + uptime);
}