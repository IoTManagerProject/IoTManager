#include "items/LoggingClass.h"

LoggingClass::LoggingClass(unsigned long period, unsigned int maxPoints, String key) {
    _period = period;
    _maxPoints = maxPoints;
    _key = key;
}

LoggingClass::~LoggingClass() {}

void LoggingClass::loop() {
    currentMillis = millis();
    unsigned long difference = currentMillis - prevMillis;
    if (difference >= _period) {
        prevMillis = millis();
        writeDate();
    }
}

void LoggingClass::writeDate() {
    SerialPrint("I", "Logging", _key);
}

MyLoggingVector* myLogging= nullptr;