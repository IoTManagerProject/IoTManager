#pragma once

#include <Arduino.h>
#include <functional>

#include <LittleFS.h>

#include "Objects/LogMetadata.h"

#define FILE_READ "r"
#define FILE_WRITE "w"

typedef std::function<bool(LogMetadata *, uint8_t *)> LogEntryHandler;

class LogReader {
   private:
    uint8_t buf[sizeof(Entry)];
    File _file;
    LogMetadata *_meta;
    LogEntryHandler _handler;
    size_t _pos;
    bool _active;

   public:
    LogReader(LogMetadata *meta, LogEntryHandler handler) : _meta{meta},
                                                            _handler{handler},
                                                            _pos{0},
                                                            _active{false} {
    }

    void setActive(bool value) {
        _file = LittleFS.open(_meta->getDataFile().c_str(), FILE_READ);
        if (_file) {
            _active = true;
        }
    }

    bool isActive() {
        return _active;
    }

    void loop() {
        if (!_active || (!_file)) {
            return;
        }

        if (_file.available()) {
            _file.readBytes((char *)&buf, sizeof(Entry));
            _handler(_meta, buf);
            _pos++;
        } else {
            _file.close();
            _active = false;
        }
    }
};