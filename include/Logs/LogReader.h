#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include <functional>

#include "Logs/LogMetadata.h"

#define FILE_READ "r"
#define FILE_WRITE "w"

class LogReader {
   private:
    File _file;
    LogEntryHandler _handler;
    bool _active;
    size_t _pos;
    LogMetadata *_meta;

   public:
    LogReader(LogMetadata &meta, LogEntryHandler handler) : _handler{handler},
                                                            _active{false},
                                                            _pos{0} {
    }

    void setActive(bool value) {
        _active = value;
    }

    void loop() {
        if (!_active) {
            return;
        }
        if (!_file) {
            _file = LittleFS.open(_meta->getDataFile().c_str(), FILE_READ);
        }
        if (_file.available()) {
            Entry entry;
            if (_file.readBytesUntil('\n', (uint8_t *)&entry, sizeof(Entry)) != sizeof(Entry)) {
                Serial.println("mismatch");
            }
            auto logEntry = LogEntry(entry, _meta->getValueType());
            _handler(logEntry);
            _pos++;

        } else {
            _file.close();
            _active = false;
        }
    }
};

