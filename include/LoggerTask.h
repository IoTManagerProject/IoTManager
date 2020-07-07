#pragma once

#include <Arduino.h>

#include "LogFile.h"

class LoggerTask {
   public:
    LoggerTask(const String& name, unsigned long period, size_t limit);
    const char* name();
    void publish();
    void update();
    void clear();
    LogMetadata* getMetadata();

   private:
    void publishFile();
    void flush();
    void post();
    void postFile();
    const String getTopic();

   private:
    std::vector<LogEntry*> _buf;
    String _name;
    unsigned long _period;
    size_t _limit;
    unsigned long _lastExecute;
    LogFile _log;
};