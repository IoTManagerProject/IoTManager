#pragma once

#include <Arduino.h>

#include <Logs/LogEntry.h>

#define FILE_READ "r"
#define FILE_WRITE "w"
#define FILE_APPEND "a"

class LogMetadata : public Printable {
   private:
    struct Data {
        char name[32];
        size_t entry_count;
        unsigned long start_time;
        unsigned long finish_time;
        ValueType_t type;

        Data() : entry_count{0}, start_time{0}, finish_time{0}, type{VT_INT} {};
        void reset() {
            entry_count = 0;
            start_time = 0;
            finish_time = 0;
        }
    };

    Data _data;

   public:
    LogMetadata(const char* name) {
        strcpy(_data.name, name);
        restore();
    };

    void reset() {
        _data.reset();
    }

    const String getDataFile() const {
        String res;
        res = "/log_";
        res += _data.name;
        res += ".txt";
        return res;
    }

    const String getMetaFile() const {
        String res;
        res = "/log_";
        res += _data.name;
        res += ".dat";
        return res;
    }

    const String getMqttTopic() const {
        String res = _data.name;
        res += "_ch";
        return res;
    }

    ValueType_t getValueType() const {
        return _data.type;
    }

    void add(const LogEntry& logEntry) {
        if (!_data.start_time) {
            _data.start_time = logEntry.data.time;
        }
        _data.finish_time = logEntry.data.time;
        _data.entry_count++;
    }

    void restore() {
        if (LittleFS.exists(getMetaFile().c_str())) {
            auto file = LittleFS.open(getMetaFile().c_str(), FILE_READ);
            file.read((uint8_t*)&_data, sizeof(_data));
            file.close();
        } else {
            Serial.println("restore meta");
        }
    }

    void store() {
        auto file = LittleFS.open(getMetaFile().c_str(), FILE_WRITE);
        if (file) {
            file.write((uint8_t*)&_data, sizeof(_data));
        } else {
            Serial.println("store meta");
        }
        file.close();
    }

    size_t getCount() const {
        return _data.entry_count;
    }

    size_t getSize() const {
        return (sizeof(Entry) + 1) * _data.entry_count;
    }

    size_t printTo(Print& p) const {
        size_t n = p.print(F("entries: "));
        n += p.print(getCount(), DEC);
        n += p.print(F(" bytes: "));
        n += p.print(getSize(), DEC);
        n += p.print(F(" start: "));
        n += p.print(_data.start_time, DEC);
        n += p.print(F(" finish: "));
        n += p.print(_data.finish_time, DEC);
        n += p.print(F(" entry_size: "));
        n += p.println(sizeof(Entry));
        return n;
    }
};
