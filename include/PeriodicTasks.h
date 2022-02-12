#pragma once
#include "Global.h"
#include "WsServer.h"
#include "Utils/TimeUtils.h"

extern void periodicTasksInit();
extern void printGlobalVarSize();

extern void handleError(String errorId, String errorValue);
extern void handleError(String errorId, int errorValue);