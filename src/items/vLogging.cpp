#include "items/vLogging.h"

#include <Arduino.h>

#include "Class/LineParsing.h"
#include "Global.h"
#include "BufferExecute.h"

LoggingClass::LoggingClass(unsigned long period, unsigned int maxPoints, String loggingValueKey, String key) {
    _period = period * 1000;
    _maxPoints = maxPoints;
    _loggingValueKey = loggingValueKey;
    _key = key;
}

LoggingClass::~LoggingClass() {}

void LoggingClass::loop() {
    currentMillis = millis();
    difference = currentMillis - prevMillis;
    if (difference >= _period) {
        prevMillis = millis();
        addNewDelOldData("logs/" + _key + ".txt", _maxPoints, getValue(_loggingValueKey)); //jsonReadStr(configLiveJson , _loggingValueKey));
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

    if (payload != "") {
        if (lines_cnt > maxPoints) {
            logData = deleteBeforeDelimiter(logData, "\r\n");
            if (timeNow->hasTimeSynced()) {
                logData += timeNow->getTimeUnix() + " " + payload + "\r\n";
                writeFile(filename, logData);
            }
        }
        else {
            if (timeNow->hasTimeSynced()) {
                addFileLn(filename, timeNow->getTimeUnix() + " " + payload);
            }
        }
    }
}

MyLoggingVector* myLogging = nullptr;

void logging() {
    myLineParsing.update();
    String loggingValueKey = myLineParsing.gval();
    String key = myLineParsing.gkey();
    String interv = myLineParsing.gint();
    String maxcnt = myLineParsing.gcnt();
    myLineParsing.clear();

    loggingKeyList += key + ",";

    static bool firstTime = true;
    if (firstTime) myLogging = new MyLoggingVector();
    firstTime = false;
    myLogging->push_back(LoggingClass(interv.toInt(), maxcnt.toInt(), loggingValueKey, key));
}

void choose_log_date_and_send() {
    String all_line = loggingKeyList;
    while (all_line.length() != 0) {
        String tmp = selectToMarker(all_line, ",");
        sendLogData("logs/" + tmp + ".txt", tmp);
        all_line = deleteBeforeDelimiter(all_line, ",");
    }
}

void sendLogData(String file, String topic) {
    String log_date = readFile(file, 5120);
    if (log_date != "failed") {
        log_date.replace("\r\n", "\n");
        log_date.replace("\r", "\n");
        String buf = "{}";
        String json_array;
        String unix_time;
        String value;
        while (log_date.length()) {
            String tmp = selectToMarker(log_date, "\n");
            log_date = deleteBeforeDelimiter(log_date, "\n");
            unix_time = selectToMarker(tmp, " ");
            jsonWriteInt(buf, "x", unix_time.toInt());
            value = deleteBeforeDelimiter(tmp, " ");
            jsonWriteFloat(buf, "y1", value.toFloat());
            if (log_date.length() < 3) {
                json_array += buf;
            }
            else {
                json_array += buf + ",";
            }
            buf = "{}";
        }
        unix_time = "";
        value = "";
        log_date = "";
        json_array = "{\"status\":[" + json_array + "]}";
        //SerialPrint("I", "module", json_array);

        publishChart(topic, json_array);
    }
}

void cleanLogAndData() {
#ifdef ESP8266
    auto dir = LittleFS.openDir("logs");
    while (dir.next()) {
        String fname = dir.fileName();
        SerialPrint("I", "System", fname);
        removeFile("logs/" + fname);
    }
#endif
    removeFile("store.json");
    configStoreJson = "";
}
