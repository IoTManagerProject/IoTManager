#include "Logger.h"

#include "LoggerTask.h"
#include "MqttClient.h"
#include "TickerScheduler/TickerScheduler.h"

//
#include <LITTLEFS.h>

std::vector<LoggerTask> _items;

static const char* MODULE = "Logger";

namespace Logger {

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