#include "items/LoggingClass.h"

LoggingClass::LoggingClass(unsigned long period, unsigned int maxPoints, String key) {
    _period = period;
    _maxPoints = maxPoints;
    _key = key;
}

LoggingClass::~LoggingClass() {}

void LoggingClass::loop() {
    currentMillis = millis();
    unsigned long difference = currentMillis - prevMillis;
    if (difference >= _period) {
        prevMillis = millis();
        addNewDelOldData("log." + _key + ".txt", _maxPoints, jsonReadStr(configLiveJson, _key));
    }
}


void LoggingClass::addNewDelOldData(const String filename, size_t maxPoints, String payload) {
    String logData = readFile(filename, 5120);
    size_t lines_cnt = itemsCount(logData, "\r\n");

    SerialPrint("I", "Logging", "http://" + WiFi.localIP().toString() + "/" + filename + " (" + String(lines_cnt, DEC) + ")");

    if ((lines_cnt > maxPoints + 1) || !lines_cnt) {
        removeFile(filename);
        lines_cnt = 0;
    }

    if (lines_cnt > maxPoints) {
        logData = deleteBeforeDelimiter(logData, "\r\n");
        if (timeNow->hasTimeSynced()) {
            logData += timeNow->getTimeUnix() + " " + payload + "\r\n";
            writeFile(filename, logData);
        }
    } else {
        if (timeNow->hasTimeSynced()) {
            addFileLn(filename, timeNow->getTimeUnix() + " " + payload);
        }
    }
}

MyLoggingVector* myLogging = nullptr;