#pragma once

#include "Arduino.h"

class LoggerTask {
   public:
    LoggerTask(String name, unsigned long period, size_t limit);
    const String getName();

    void publish();
    void update();
    void clear();

   private:
    void post();
    const String getTopic();
    const String getFilename();

   private:
    String _name;
    unsigned long _period;
    size_t _limit;
    unsigned long _lastExecute;
};