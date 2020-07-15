#pragma once

#include <Arduino.h>
#include <FS.h>

#include "Base/Value.h"

struct Entry {
   private:
    unsigned long _time;
    unsigned short _value;

   public:
    Entry() : _time{0}, _value{0} {}

    Entry(unsigned long time, float value) {
        _time = time;
        _value = (unsigned short)(value * 100);
    }

    unsigned long getTime() const {
        return _time;
    }

    float getValue() const {
        return (float)(_value / 100);
    }
};

struct LogEntry {
    Entry data;

    LogEntry(){};

    LogEntry(const Entry entry) : data{entry} {}

    LogEntry(uint8_t* ptr) {
        memcpy(&data, ptr, sizeof(Entry));
    }

    LogEntry(const String& str) {
        size_t split = str.indexOf(" ");
        unsigned long time = str.substring(0, split).toInt();
        float value = str.substring(split).toFloat();
        data = Entry(time, value);
    }

    LogEntry(unsigned long time, float value) {
        data = Entry(time, value);
    };

    const String asChartEntry(uint8_t num = 1) const {
        String str = "{\"x\":";
        str += String(data.getTime(), DEC);
        str += ",\"y";
        str += String(num, DEC);
        str += "\":";
        str += String(data.getValue(), 2);
        str += "}";
        return str;
    }

    unsigned long getTime() const {
        return data.getTime();
    }

    float getValue() const {
        return data.getValue();
    }

    void write(Print& p) {
        p.write((uint8_t*)&data, sizeof(Entry));
    }
};
