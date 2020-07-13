#pragma once

#include <Arduino.h>

#include <Logs/LogEntry.h>

struct LogMetadata {
    size_t entry_count;
    unsigned long start_time;
    unsigned long finish_time;

    LogMetadata() : entry_count{0}, start_time{0}, finish_time{0} {};

    void add(LogEntry entry) {
        if (!entry_count) {
            start_time = entry.time();
        }
        finish_time = entry.time();
        entry_count++;
    }

    size_t size() const {
        return sizeof(LogEntry) * entry_count;
    }

    void print(Print& p) const {
        p.print(F("entries: "));
        p.print(entry_count);
        p.print(F(" bytes: "));
        p.print(size());
        p.print(F(" start: "));
        p.print(start_time);
        p.print(F(" finish: "));
        p.print(finish_time);
        p.print(F(" entry_size: "));
        p.println(sizeof(LogEntry));
    }
};
