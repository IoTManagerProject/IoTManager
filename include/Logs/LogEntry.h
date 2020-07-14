#pragma once

#include <Arduino.h>
#include <FS.h>

#include "Base/Value.h"

struct Entry {
    unsigned long time;
    unsigned short value;
};

struct LogEntry {
    Entry data;
    ValueType_t _type;

    LogEntry(){};

    LogEntry(const Entry& entry, ValueType_t type) {
        data.time = entry.time;
        data.value = type == VT_FLOAT ? entry.value / 100 : entry.value;
        _type = type;
    }

    LogEntry(const String& str) {
        size_t split = str.indexOf(" ");
        data.time = str.substring(0, split).toInt();
        data.value = str.substring(split).toInt();
        _type = VT_INT;
    }

    LogEntry(unsigned long time, const String& value, ValueType_t type) {
        _type = type;
        data.time = time;
        if (_type == VT_INT) {
            data.value = value.toInt();
        } else if (_type == VT_FLOAT) {
            data.value = value.toFloat() * 100;
        } else if (_type == VT_STRING) {
            data.value = 777;
        }
    };

    const String asChartEntry(uint8_t num = 1) const {
        String str = "{\"x\":";
        str += getTime();
        str += ",\"y";
        str += String(num, DEC);
        str += "\":";
        str += getValue();
        str += "}";
        return str;
    }

    const String getTime() const {
        return String(data.time, DEC);
    }

    const String getValue() const {
        return VT_INT ? String(data.value, DEC) : String((float)data.value / 100, 2);
    }

    void write(Print& p) {
        p.write((uint8_t*)&data, sizeof(Entry));
        p.write('\n');
    }

    void print(Print& p) const {
        p.print(getTime());
        p.print(' ');
        p.println(getValue());
    }
};
