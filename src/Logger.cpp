#include "Logger.h"

#include "LoggerTask.h"
#include "MqttClient.h"
#include "TickerScheduler/TickerScheduler.h"
#include "Utils/StringUtils.h"

#include <LITTLEFS.h>

namespace Logger {
std::vector<LoggerTask> _list;

class LogReader {
   private:
    File _file;
    EntryHandler _handler;
    bool _active;
    size_t _pos;
    LoggerTask* _task;

   public:
    LogReader(LoggerTask* task, EntryHandler handler) : _handler{handler},
                                                        _active{false},
                                                        _pos{0},
                                                        _task{task} {
        _file = LittleFS.open(task->getDataFile(), "r");
    }

    void loop() {
        if (!_active) {
            return;
        }
        if (!_file) {
            _active = false;
        }
        if (_file.available()) {
            String line = _file.readStringUntil('\r');
            _pos++;
            size_t split = line.indexOf(" ");
            _handler(_task->getName(), line.substring(0, split).toInt(), line.substring(split).toFloat());
        } else {
            _file.close();
            _active = false;
        }
    }

    void setActive(bool value) {
        _active = value;
    }
};

LogReader* _reader;

const String asJson() {
    String res = "{\"logs\":[";
    for (size_t i = 0; i < _list.size(); i++) {
        auto* item = &_list.at(i);
        res += "{\"id\":\"";
        res += String(i, DEC);
        res += "\",";
        res += "\"name\":\"";
        res += item->getName();
        res += "\",";
        res += "\"entries\":\"";
        res += String(item->getMetadata()->_entries, DEC);
        res += "\",";
        res += "\"size\":\"";
        res += prettyBytes(item->getMetadata()->_bytes);
        res += "\"";
        res += i < _list.size() - 1 ? "}," : "}";
    }
    res += "]}";
    return res;
}

void loop() {
    if (_reader)
        _reader->loop();
    else
        for (size_t i = 0; i < _list.size(); i++) {
            _list.at(i).update();
        }
}

void add(String name, unsigned long period, size_t limit) {
    _list.push_back(LoggerTask{name, period, limit});
}

void init() {
    _list.clear();
}

void publishLog(LoggerTask* task, EntryHandler func) {
    if (_reader) {
        delete _reader;
    }
    _reader = new LogReader(task, func);
    _reader->setActive(true);
}

void publishTasks(EntryHandler func) {
    for (auto item : _list) {
        publishLog(&item, func);
        break;
    }
}

void clear() {
    for (size_t i = 0; i < _list.size(); i++) {
        _list.at(i).clear();
    }
}

}  // namespace Logger