#pragma once

#include <Arduino.h>

#include <Base/Value.h>

struct LogEntry;

typedef std::function<bool(LogEntry)> LogEntryHandler;

struct LogEntry : public Printable {
    unsigned long _time;
    unsigned short _value;
    ValueType_t _type;

    LogEntry(const String& str) {
        size_t split = str.indexOf(" ");
        _time = str.substring(0, split).toInt();
        _value = str.substring(split).toFloat();
    }

    LogEntry(unsigned long time, const String& value, ValueType_t type) : _time{time}, _type{type} {
        if (_type == VT_INT) {
            _value = value.toInt();
        } else if (_type == VT_FLOAT) {
            _value = value.toFloat() * 100;
        } else if (_type == VT_STRING) {
            _value = 777;
        }
    };

    unsigned long time() {
        return _time;
    }

    const String asChartEntry(uint8_t num = 1) const {
        String str = "[\"x\":\"";
        str += _time;
        str += "\",\"y";
        str += String(num, DEC);
        str += ":\"";
        str += getValue();
        str += "\"]";
        return str;
    }

    const String getValue() const {
        return VT_INT ? String(_value, DEC) : String((float)_value / 100, 2);
    }

    size_t printTo(Print& p) const override {
        size_t res = p.print(_time);
        res += p.print(' ');
        res += p.println(getValue());
        return res;
    }
};
