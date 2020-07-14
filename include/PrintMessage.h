#pragma once

#include <Arduino.h>

#define pm PrintMessage(MODULE)

enum ErrorLevel_t {
    EL_INFO,
    EL_WARNING,
    EL_ERROR
};

class PrintMessage {
   public:
    PrintMessage(const char* module);

    void error(const String str);
    void info(const String str);

   public:
    static void setOutput(Print* out);
    static Print* getOutput();
    static void enablePrint(bool value);
    static void enableLog(bool value);

   private:
    void print(ErrorLevel_t level, const String& str);
    void print(const char* level, const char* str);
    void print(const char* module, const char* level, const char* str);

    const String getTimeToken();

   private:
    static const char* getErrorLevelStr(uint8_t level);
    static void print(const char* time, const char* module, const char* level, const char* str);

   private:
    const char* _module;

   private:
    static const char* error_levels[];
    static bool printEnabled;
    static bool logEnabled;
    static Print* out;
};
