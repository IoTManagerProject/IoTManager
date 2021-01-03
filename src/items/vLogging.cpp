#include "items/vLogging.h"

#include <Arduino.h>

#include "BufferExecute.h"
#include "Class/LineParsing.h"
#include "FileSystem.h"
#include "Global.h"

LoggingClass::LoggingClass(unsigned long period, unsigned int maxPoints, String loggingValueKey, String key) {
    _period = period * 1000;
    _maxPoints = maxPoints;
    _loggingValueKey = loggingValueKey;
    _key = key;
}

LoggingClass::~LoggingClass() {}

void LoggingClass::loop() {
    if (_period > 0) {
        currentMillis = millis();
        difference = currentMillis - prevMillis;
        if (difference >= _period) {
            prevMillis = millis();
            execute("");
        }
    }
}

void LoggingClass::execute(String keyOrValue) {
    String loggingValue = "";

    if (keyOrValue == "") {  //прилетело из лупа
        if (getValue(_loggingValueKey) != "no value") {
            loggingValue = getValue(_loggingValueKey);
        } else {
            SerialPrint("E", "Logging", "This value not found on this device");
        }
    } else {                                   //прилетело из события
        if (isDigitDotCommaStr(keyOrValue)) {  //если это число или дробное число
            loggingValue = keyOrValue;
        } else {  //если это ключ
            if (getValue(_loggingValueKey) != "no value") {
                loggingValue = getValue(keyOrValue);
            } else {
                SerialPrint("E", "Logging", "This value not found on this device");
            }
        }
    }

    String filename = "logs/" + _key + ".txt";

    size_t cnt = countLines(filename);
    size_t sz = getFileSize(filename);

    SerialPrint("I", "Logging", "http://" + WiFi.localIP().toString() + "/" + filename + " lines " + String(cnt, DEC) + ", size " + String(sz));

    if ((cnt > _maxPoints + 1)) {
        removeFile(filename);
        SerialPrint("E", "Logging", "file been remooved: " + filename + " " + String(cnt) + ">" + String(_maxPoints));
        cnt = 0;
    }

    if (loggingValue != "") {
        if (cnt >= _maxPoints) {                         //удаляем старую строку и добавляем новую
            String logData = readFile(filename, 20480);  //10240
            SerialPrint("I", "Logging", "Free heap " + String(ESP.getFreeHeap()));
            if (logData == "large") {
                SerialPrint("E", "Logging", "File is very large");
            }

            logData = deleteBeforeDelimiter(logData, "\r\n");

            if (timeNow->hasTimeSynced()) {
                logData += timeNow->getTimeUnix() + " " + loggingValue + "\r\n";

                writeFile(filename, logData);
            }
        } else {  //просто добавляем новую строку
            if (timeNow->hasTimeSynced()) {
                addFileLn(filename, timeNow->getTimeUnix() + " " + loggingValue);
            }
        }
    }

    String buf = "{}";
    jsonWriteInt(buf, "x", timeNow->getTimeUnix().toInt());
    jsonWriteFloat(buf, "y1", loggingValue.toFloat());
    buf = "{\"status\":[" + buf + "]}";
    publishChart(_key, buf);
}

MyLoggingVector* myLogging = nullptr;

void logging() {
    myLineParsing.update();
    String loggingValueKey = myLineParsing.gval();
    String key = myLineParsing.gkey();
    String interv = myLineParsing.gint();
    String maxcnt = myLineParsing.gcnt();
    myLineParsing.clear();

    logging_KeyList += key + ",";

    logging_EnterCounter++;
    addKey(key, logging_KeyList, logging_EnterCounter);

    static bool firstTime = true;
    if (firstTime) myLogging = new MyLoggingVector();
    firstTime = false;
    myLogging->push_back(LoggingClass(interv.toInt(), maxcnt.toInt(), loggingValueKey, key));

    sCmd.addCommand(key.c_str(), loggingExecute);
}

void loggingExecute() {
    String key = sCmd.order();
    String value = sCmd.next();
    int number = getKeyNum(key, logging_KeyList);
    if (myLogging != nullptr) {
        if (number != -1) {
            myLogging->at(number).execute(value);
        }
    }
}

void choose_log_date_and_send() {
    String all_line = logging_KeyList;
    while (all_line.length() != 0) {
        String tmp = selectToMarker(all_line, ",");
        sendLogData("logs/" + tmp + ".txt", tmp);
        all_line = deleteBeforeDelimiter(all_line, ",");
    }
}

void sendLogData(String file, String topic) {
    File configFile = FileFS.open(file, "r");
    if (!configFile) {
        return;
    }
    configFile.seek(0, SeekSet);
    int i = 0;
    String buf = "{}";
    String json_array;
    String unix_time;
    String value;
    unsigned int psn;
    unsigned int sz = configFile.size();
    do {
        i++;
        psn = configFile.position();
        String line = configFile.readStringUntil('\n');
        unix_time = selectToMarker(line, " ");
        jsonWriteInt(buf, "x", unix_time.toInt());
        value = deleteBeforeDelimiter(line, " ");
        jsonWriteFloat(buf, "y1", value.toFloat());
        if (unix_time != "" || value != "") {
            json_array += buf + ",";
        }
        int grafmax = jsonReadInt(configSetupJson, "grafmax");
        if (grafmax != 0) {
            if (i >= grafmax) {
                json_array = "{\"status\":[" + json_array + "]}";
                json_array.replace("},]}", "}]}");
                publishChart(topic, json_array);
                json_array = "";
                i = 0;
            }
        }
    } while (psn < sz);

    configFile.close();

    json_array = "{\"status\":[" + json_array + "]}";
    json_array.replace("},]}", "}]}");
    publishChart(topic, json_array);
}

void cleanLogAndData() {
#ifdef ESP8266
    auto dir = FileFS.openDir("logs");
    while (dir.next()) {
        String fname = dir.fileName();
        SerialPrint("I", "System", fname);
        removeFile("logs/" + fname);
    }
#endif
    removeFile("store.json");
    configStoreJson = "";
}
