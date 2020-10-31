#pragma once
#include <Arduino.h>

#include "Global.h"

class LoggingClass;

typedef std::vector<LoggingClass> MyLoggingVector;

class LoggingClass {
   public:
    LoggingClass(unsigned long period, unsigned int maxPoints, String key);
    ~LoggingClass();

    void loop();
    void writeDate();

   private:
    unsigned long currentMillis;
    unsigned long prevMillis;
    unsigned long difference;

    unsigned long _period;
    unsigned int _maxPoints;
    String _key;
};

extern MyLoggingVector* myLogging;
