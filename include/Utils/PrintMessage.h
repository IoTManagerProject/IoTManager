#pragma once

#include "Clock.h"
#include "Utils\StringUtils.h"
#include "Utils\TimeUtils.h"
#include "Errors.h"

#define pm PrintMessage(MODULE)

class PrintMessage {
   public:
    PrintMessage(const char* module) : _module{module},
                                       _printEnabled{true},
                                       _logEnabled{true},
                                       _out{&Serial} {};

    void error(const String str) {
        print(EL_ERROR, str);
    }

    void info(Printable* p) {
        print(EL_INFO, p);
    }

    void info(const String str) {
        print(EL_INFO, str);
    }

    void setOutput(Print* out) {
        _out = out;
    }

    Print* getOutput() {
        return _out;
    }

    void enablePrint(bool value) {
        _printEnabled = value;
    }

    void enableLog(bool value) {
        _logEnabled = value;
    }

   private:
    void print(const ErrorLevel_t level, Printable* obj) {
        char buf[256];
        String str;
        if (now.hasSynced()) {
            str = now.getTime();
        } else {
            str = prettyMillis();
        }
        snprintf(buf, sizeof(buf), "%s [%s] [%s]", str.c_str(), getErrorLevelStr(level), _module);
        if (_printEnabled) {
            if (_out) {
                _out->print(buf);
                obj->printTo(*_out);
            }
        }
    }

    void print(const ErrorLevel_t level, const String& str) {
        char buf[256];
        snprintf(buf, sizeof(buf), "%s [%s] [%s] %s", (now.hasSynced() ? now.getTime() : now.getUptime()).c_str(), getErrorLevelStr(level), _module, str.c_str());
        if (_printEnabled) {
            if (_out) {
                _out->println(buf);
            };
        }
    }

   private:
    const char* _module;
    bool _printEnabled;
    bool _logEnabled;
    Print* _out;
};
