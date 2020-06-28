#pragma once

#include "Arduino.h"
#include "Utils\StringUtils.h"
#include "Utils\TimeUtils.h"
#include "Errors.h"

#define pm PrintMessage(MODULE)

class PrintMessage {
   public:
    PrintMessage(const char* module) {
        _module = module;
    }

    void error(const String str) {
        print(EL_ERROR, str);
    }

    void info(const String str) {
        print(EL_INFO, str);
    }

   private:
    void print(const ErrorLevel_t level, const String& str) {
        Serial.printf("%s [%s] [%s] %s\n", prettyMillis(millis()).c_str(), getErrorLevelStr(level).c_str(), _module, str.c_str());
    }

   private:
    const char* _module;
};
