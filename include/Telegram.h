#pragma once
#include "Global.h"

extern void sendTelegramMsg();
extern void telegramInit();
extern void handleTelegram();
extern bool isTelegramEnabled();
extern void telegramMsgParse(String msg);
extern String returnListOfParams();