#ifdef EnableLogging
#pragma once
#include "Consts.h"
#include <Arduino.h>

#include "Global.h"

class LoggingClass;

typedef std::vector<LoggingClass> MyLoggingVector;

class LoggingClass {
   public:

    LoggingClass(String interval, unsigned int maxPoints, String loggingValueKey, String key, String startState, bool savedFromWeb);
    ~LoggingClass();

    void loop();
    void execute(String keyOrValue);

   private:

    unsigned long currentMillis;
    unsigned long prevMillis;
    unsigned long difference;
    String _interval;
    unsigned int _intervalSec;
    unsigned int _type = 0;
    unsigned int _maxPoints;
    String _loggingValueKey;
    String _key;

    
};

extern MyLoggingVector* myLogging;

extern void logging();
extern void loggingExecute();
extern void choose_log_date_and_send();
extern void sendLogData(String file, String topic);
extern void sendLogData2(String file, String topic);
extern void cleanLogAndData();
#endif
