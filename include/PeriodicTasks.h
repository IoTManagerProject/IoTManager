#pragma once
#include "Global.h"
#include "WsServer.h"
#include "utils/TimeUtils.h"

#ifdef ESP32
#include <rom/rtc.h>
#endif

extern void periodicTasksInit();
extern void printGlobalVarSize();

extern String ESP_getResetReason(void);
extern String ESP32GetResetReason(uint32_t cpu_no);