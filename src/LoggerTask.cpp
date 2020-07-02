#include "LoggerTask.h"

#include "Global.h"
#include "MqttClient.h"

static const char* MODULE = "LoggerTask";

LoggerTask::LoggerTask(String name, unsigned long period, size_t limit) {
    _name = name;
    _period = period;
    _limit = limit;
    _lastExecute = 0;
}

const String LoggerTask::getName() {
    return _name;
}
const String LoggerTask::getFilename() {
    return "log." + _name + ".txt";
}

void LoggerTask::update() {
    if (millis_since(_lastExecute) >= _period) {
        post();
        _lastExecute = millis();
    }
}

void LoggerTask::clear() {
    removeFile(getFilename().c_str());
}

void LoggerTask::post() {
    pm.info("task: " + _name);
    String value = liveData.read(_name);
    String filename = getFilename();
    String buf;
    if (!readFile(filename.c_str(), buf, 5120)) {
        return;
    }
    size_t lines_cnt = itemsCount(buf, "\r\n");

    pm.info("log " + filename + " (" + String(lines_cnt, DEC) + ")");

    if ((lines_cnt > _limit + 1) || !lines_cnt) {
        removeFile(filename);
        lines_cnt = 0;
    }

    if (lines_cnt > _limit) {
        buf = deleteBeforeDelimiter(buf, "\r\n");
        if (timeNow->hasTimeSynced()) {
            buf += timeNow->getTimeUnix() + " " + value + "\r\n";
            writeFile(filename, buf);
        }
    } else {
        if (timeNow->hasTimeSynced()) {
            addFile(filename, timeNow->getTimeUnix() + " " + value);
        }
    }
}

const String LoggerTask::getTopic() {
    return _name + "_ch";
}

void LoggerTask::publish() {
    String filename = getFilename();
    String data;
    if (!readFile(filename.c_str(), data, 5120)) {
        pm.error("read " + filename);
        return;
    }
    data.replace("\r\n", "\n");
    data.replace("\r", "\n");
    String buf = "{}";
    String json_array;
    String unix_time;
    String value;
    while (data.length()) {
        String tmp = selectToMarker(data, "\n");
        data = deleteBeforeDelimiter(data, "\n");
        unix_time = selectToMarker(tmp, " ");
        jsonWriteInt(buf, "x", unix_time.toInt());
        value = deleteBeforeDelimiter(tmp, " ");
        jsonWriteFloat(buf, "y1", value.toFloat());
        if (data.length() < 3) {
            json_array += buf;
        } else {
            json_array += buf + ",";
        }
        buf = "{}";
    }
    unix_time = "";
    value = "";
    data = "";
    json_array = "{\"status\":[" + json_array + "]}";
    pm.info(json_array);

    MqttClient::publishChart(getTopic().c_str(), json_array);
}
