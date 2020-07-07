#include "Logger.h"

#include "LoggerTask.h"
#include "MqttClient.h"
#include "TickerScheduler/TickerScheduler.h"
#include "Utils/StringUtils.h"

//
#include <LITTLEFS.h>

std::vector<LoggerTask> _items;

namespace Logger {

const String asJson() {
    String res = "{\"logs\":[";
    for (size_t i = 0; i < _items.size(); i++) {
        auto* item = &_items.at(i);
        res += "{\"id\":\"";
        res += String(i, DEC);
        res += "\",";
        res += "\"name\":\"";
        res += item->name();
        res += "\",";
        res += "\"entries\":\"";
        res += String(item->getMetadata()->_entries, DEC);
        res += "\",";
        res += "\"size\":\"";
        res += prettyBytes(item->getMetadata()->_bytes);
        res += "\"";
        res += i < _items.size() - 1 ? "}," : "}";
    }
    res += "]}";
    return res;
}

void loop() {
    for (size_t i = 0; i < _items.size(); i++) {
        _items.at(i).update();
    }
}

void add(String name, unsigned long period, size_t limit) {
    _items.push_back(LoggerTask{name, period, limit});
}

void init() {
    _items.clear();
}

void publish() {
    for (size_t i = 0; i < _items.size(); i++) {
        _items.at(i).publish();
    }
}

void clear() {
    for (size_t i = 0; i < _items.size(); i++) {
        _items.at(i).clear();
    }
}

}  // namespace Logger