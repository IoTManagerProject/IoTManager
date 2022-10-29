#pragma once
#include "Global.h"
#include "utils/TimeUtils.h"
#include "classes/IoTItem.h"

void SerialPrint(const String& errorLevel, const String& module, const String& msg, const String& itemId = "");