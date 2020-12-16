#pragma once
#include "Consts.h"
#ifdef telegramEnable
#include "Global.h"

extern void sendTelegramMsg();
extern void telegramInit();
extern void handleTelegram();
extern bool isTelegramEnabled();
extern bool isTelegramInputOn();
extern void telegramMsgParse(String msg);
extern String returnListOfParams();
#endif