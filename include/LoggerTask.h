#pragma once

#include "Arduino.h"

struct LogEntry {
    unsigned long time;
    unsigned short value;
    LogEntry(unsigned long time, uint16_t value) : time{time}, value{value} {};
};

class LoggerTask {
   public:
    LoggerTask(const String& name, unsigned long period, size_t limit);
    const char* name();
    void publish();
    void update();
    void clear();

   private:
    void publishFile();
    void flush();
    void post();
    void postFile();
    const String getTopic();

   private:
    std::vector<LogEntry> _buf;
    String _name;
    char _filename[32];
    unsigned long _period;
    size_t _limit;
    size_t _records;
    unsigned long _lastExecute;
};