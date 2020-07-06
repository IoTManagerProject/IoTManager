#include "LoggerTask.h"

#include "Global.h"
#include "MqttClient.h"

static const char* MODULE = "LoggerTask";

LoggerTask::LoggerTask(const String& name, unsigned long period, size_t limit) {
    _name = name;
    strcpy(_filename, "log.");
    strcat(_filename, _name.c_str());
    strcat(_filename, ".txt");
    _period = period;
    _limit = limit;
    _buf.reserve(_limit);
    _records = 0;
    _lastExecute = 0;
}

const char* LoggerTask::name() {
    return _name.c_str();
}

void LoggerTask::update() {
    if (millis_since(_lastExecute) >= _period) {
        if (timeNow->hasTimeSynced()) {
            if (_buf.size() >= _limit) {
                flush();
            }
            _buf.push_back(
                LogEntry(timeNow->getEpoch(), liveData.readInt(_name)));

            pm.info("buf " + String(_buf.size(), DEC) + " of " + String(_limit, DEC));
        }
        _lastExecute = millis();
    }
}

void LoggerTask::flush() {
    auto file = LittleFS.open(_filename, "a");
    for (auto item : _buf) {
        char payload[64];
        char buf[16];
        strcpy(payload, itoa(item.time, buf, DEC));
        strcat(payload, " ");
        strcat(payload, itoa(item.value, buf, DEC));
        pm.info(payload);
        file.println(payload);
    }
    file.close();
    _buf.clear();
}

void LoggerTask::publish() {
}

void LoggerTask::clear() {
    removeFile(_filename);
}

void LoggerTask::postFile() {
    pm.info("task: " + _name);
    String value = liveData.read(_name);
    String buf;
    if (readFile(_filename, buf, 5120)) {
        size_t lines_cnt = itemsCount(buf, "\r\n");
        if ((lines_cnt > _limit + 1) || !lines_cnt) {
            removeFile(_filename);
            lines_cnt = 0;
        }
        if (lines_cnt > _limit) {
            buf = deleteBeforeDelimiter(buf, "\r\n");
            if (timeNow->hasTimeSynced()) {
                buf += timeNow->getTimeUnix() + " " + value + "\r\n";
                writeFile(_filename, buf);
            }
        }
    }
    if (timeNow->hasTimeSynced()) {
        addFile(_filename, timeNow->getTimeUnix() + " " + value);
    }
}

const String LoggerTask::getTopic() {
    return _name + "_ch";
}

void LoggerTask::publishFile() {
    String data;
    if (!readFile(_filename, data, 5120)) {
        pm.error("open file");
        return;
    }
    data.replace("\r\n", "\n");
    data.replace("\r", "\n");
    String buf = "{}";
    String json_array;
    String unix_time;
    String value;
    while (!data.isEmpty()) {
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
