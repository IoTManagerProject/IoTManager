#pragma once

#include "Arduino.h"
#include "Utils/StringUtils.h"
#include "Utils/TimeUtils.h"
#include "Errors.h"
#include "Global.h"

#define pm PrintMessage(MODULE)

class PrintMessage {
   public:
    PrintMessage(const char* module) {
        _module = module;
    }

    void error(const String& str) {
        print(EL_ERROR, str);
    }

    void info(const String& str) {
        print(EL_INFO, str);
    }

   private:
    void printErrorLevel(ErrorLevel_t level) {
        Serial.printf("[%c] ", getErrorLevelStr(level));
    }

    void printUptime() {
        Serial.printf("%lu ", ((unsigned long)millis() / 1000));
    }

    void printModule() {
        Serial.printf("[%s] ", _module);
    }

    void print(const ErrorLevel_t level, const String& str) {
        printUptime();
        printErrorLevel(level);
        printModule();
        Serial.println(str.c_str());
    }

   private:
    const char* _module;
};
