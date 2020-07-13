#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include <functional>

#include "Logs/LogEntry.h"

class LogReader {
   private:
    File _file;
    LogEntryHandler _handler;
    bool _active;
    size_t _pos;

   public:
    LogReader(const char* filename, LogEntryHandler handler) : _handler{handler},
                                                               _active{false},
                                                               _pos{0} {
        _file = LittleFS.open(filename, "r");
    }

    void setActive(bool value) {
        _active = value;
    }

    void loop() {
        if (!_active) {
            return;
        }
        if (!_file) {
            _active = false;
        }
        if (_file.available()) {
            String line = _file.readStringUntil('\n');
            Serial.println(line);
            _handler(LogEntry(line));
            _pos++;

        } else {
            _file.close();
            _active = false;
        }
    }
};
