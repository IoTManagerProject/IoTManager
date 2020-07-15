#include "PrintMessage.h"

#include "Clock.h"
#include "MqttClient.h"
#include "Utils\StringUtils.h"
#include "Utils\TimeUtils.h"

const char* PrintMessage::error_levels[] = {"I", "W", "E", "?"};
bool PrintMessage::mqttEnabled = false;
bool PrintMessage::fileEnabled = false;
bool PrintMessage::printEnabled = true;
Print* PrintMessage::out = &Serial;

void PrintMessage::setOutput(Print* output) {
    out = output;
}

Print* PrintMessage::getOutput() {
    return out;
}

void PrintMessage::enablePrint(bool value) {
    printEnabled = value;
}

void PrintMessage::enableLog(bool value) {
    fileEnabled = value;
}

const char* PrintMessage::getErrorLevelStr(uint8_t level) {
    return error_levels[level];
}

const String PrintMessage::getTimeToken() {
    return now.hasSynced() ? now.getTime() : now.getUptime();
};

void PrintMessage::print(const char* time, const char* level, const char* module, const char* str) {
    char buf[256];

    if (mqttEnabled) {
        snprintf(buf, 256, "%s %s", module, str);
        auto writer = MqttClient::getWriter("syslog");
        writer->print(buf);
        delete writer;
    }

    if (fileEnabled) {
        //
    }
    if (printEnabled) {
        snprintf(buf, 256, "%s [%s] [%s] %s",
                 time, level, module, str);
        if (out) {
            out->println(buf);
        };
    }
}

PrintMessage::PrintMessage(const char* module) : _module{module} {};

void PrintMessage::error(const String str) {
    print(EL_ERROR, str);
}

void PrintMessage::info(const String str) {
    print(EL_INFO, str);
}

void PrintMessage::print(ErrorLevel_t level, const String& str) {
    print(getErrorLevelStr(level), str.c_str());
}

void PrintMessage::print(const char* level, const char* str) {
    print(level, _module, str);
}

void PrintMessage::print(const char* level, const char* module, const char* str) {
    print(getTimeToken().c_str(), level, module, str);
}
