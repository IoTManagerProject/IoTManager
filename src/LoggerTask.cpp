#include "LoggerTask.h"

#include "Global.h"
#include "MqttClient.h"
#include "LogFile.h"

static const char* MODULE = "LoggerTask";

LoggerTask::LoggerTask(const String& name, unsigned long period, size_t limit) : _name{name},
                                                                                 _period{period},
                                                                                 _limit{limit},
                                                                                 _lastExecute{0},
                                                                                 _log{_name.c_str()} {
    _buf.reserve(_limit);
};

void LoggerTask::flush() {
    _log.open();
    for (size_t i = 0; i < _buf.size(); i++) {
        _log.write(_buf.at(i));
    }
    _log.close();
    _buf.clear();
}

LogMetadata* LoggerTask::getMetadata() {
    return _log.metadata();
}

const char* LoggerTask::name() {
    return _name.c_str();
}

void LoggerTask::update() {
    if (millis_since(_lastExecute) >= _period) {
        if (timeNow->hasSynced()) {
            if (_buf.size() >= _limit) {
                flush();
                _buf.clear();
            }
            _buf.push_back(new LogEntry(timeNow->getEpoch(), liveData.readInt(_name)));
            pm.info(String(_buf.size(), DEC) + " / " + String(_limit, DEC));
        }
        _lastExecute = millis();
    }
}

void LoggerTask::publish() {
}

void LoggerTask::postFile() {
    // pm.info("task: " + _name);
    // String value = liveData.read(_name);
    // String buf;
    // if (readFile(_filename, buf, 5120)) {
    //     size_t lines_cnt = itemsCount(buf, "\r\n");
    //     if ((lines_cnt > _limit + 1) || !lines_cnt) {
    //         removeFile(_filename);
    //         lines_cnt = 0;
    //     }
    //     if (lines_cnt > _limit) {
    //         buf = deleteBeforeDelimiter(buf, "\r\n");
    //         if (timeNow->hasSynced()) {
    //             buf += timeNow->getTimeUnix() + " " + value + "\r\n";
    //             writeFile(_filename, buf);
    //         }
    //     }
    // }
    // if (timeNow->hasSynced()) {
    //     addFile(_filename, timeNow->getTimeUnix() + " " + value);
    // }
}

const String LoggerTask::getTopic() {
    return _name + "_ch";
}
void LoggerTask::clear() {
}
void LoggerTask::publishFile() {
    // String data;
    // if (!readFile(_filename, data, 5120)) {
    //     pm.error("open file");
    //     return;
    // }
    // data.replace("\r\n", "\n");
    // data.replace("\r", "\n");
    // String buf = "{}";
    // String json_array;
    // String unix_time;
    // String value;
    // while (!data.isEmpty()) {
    //     String tmp = selectToMarker(data, "\n");
    //     data = deleteBeforeDelimiter(data, "\n");
    //     unix_time = selectToMarker(tmp, " ");
    //     jsonWriteInt(buf, "x", unix_time.toInt());
    //     value = deleteBeforeDelimiter(tmp, " ");
    //     jsonWriteFloat(buf, "y1", value.toFloat());
    //     if (data.length() < 3) {
    //         json_array += buf;
    //     } else {
    //         json_array += buf + ",";
    //     }
    //     buf = "{}";
    // }
    // unix_time = "";
    // value = "";
    // data = "";
    // json_array = "{\"status\":[" + json_array + "]}";
    // pm.info(json_array);

    // MqttClient::publishChart(getTopic().c_str(), json_array);
}
