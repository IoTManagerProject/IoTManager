#include "LoggerTask.h"

#include "Global.h"
#include "MqttClient.h"

static const char* MODULE = "LoggerTask";

LoggerTask::LoggerTask(const String& name, unsigned long period, size_t limit) : _period{period},
                                                                                 _limit{limit},
                                                                                 _lastExecute{0} {
    strcpy(_name, name.c_str());
    _buf.reserve(_limit);

    String metafile = getMetaFilename();
    if (LittleFS.exists(metafile)) {
        auto file = LittleFS.open(metafile, FILE_READ);
        file.read((uint8_t*)&_meta, sizeof(_meta));
        file.close();
    }
}

const String LoggerTask::getName() const {
    return _name;
}

const String LoggerTask::getTopic() {
    char buf[32];
    strcpy(buf, _name);
    strcat(buf, "_ch");
    return buf;
}

const String LoggerTask::getMetaFilename() const {
    char buf[32];
    strcpy(buf, "log_");
    strcat(buf, _name);
    strcat(buf, ".dat");
    return buf;
}

const String LoggerTask::getDataFile() const {
    char buf[32];
    strcpy(buf, "log_");
    strcat(buf, _name);
    strcat(buf, ".txt");
    return buf;
}

LogMetadata* LoggerTask::getMetadata() {
    return &_meta;
}

void LoggerTask::clear() {
    removeFile(getDataFile().c_str());
}

void LoggerTask::storeMeta() {
    auto file = LittleFS.open(getMetaFilename(), FILE_WRITE);
    if (file) {
        file.write((uint8_t*)&_meta, sizeof(_meta));
    } else {
        pm.error("store meta");
    }
    file.close();
}

void LoggerTask::update() {
    if (_buf.size() >= _limit) {
        auto file = LittleFS.open(getDataFile().c_str(), "a");
        if (file) {
            for (auto entry : _buf) {
                size_t bytes = entry.printTo(file);
                _meta.add(bytes, entry.time());
            }
            _buf.clear();
            file.close();
            storeMeta();
        }
    }

    if (millis_since(_lastExecute) >= _period) {
        if (timeNow->hasSynced()) {
            _buf.push_back(LogEntry(timeNow->getEpoch(), liveData.readInt(_name)));
            pm.info(String(_buf.size(), DEC) + " / " + String(_limit, DEC));
        }
        _lastExecute = millis();
    }
}

void LoggerTask::publish() {
}

void LoggerTask::postFile() {
    // pm.info("task: " + _name);
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

void LoggerTask::publishFile() {
    // String data;
    // if (!LittleFS.open(_ma,e, "r") {
    //     pm.error("open file");
    //     return;
    // }
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
