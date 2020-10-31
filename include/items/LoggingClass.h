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

   private:

    unsigned long currentMillis;
    unsigned long prevMillis;
    unsigned long _period;
    unsigned int _maxPoints;
    String _key;

    void addNewDelOldData(const String filename, size_t maxPoints, String payload);
};

extern MyLoggingVector* myLogging;
