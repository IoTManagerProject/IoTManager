#pragma once
#include "Global.h"
#include "Utils/TimeUtils.h"

enum module_t : uint8_t {
    M_WIFI,
    M_MQTT,
    M_WEB_SERVER,
    M_SCENARIO
};

void SerialPrint(char errorLevel, const String& module, const String& msg);

void SerialPrint(char errorLevel, module_t module, const String& msg);
