#pragma once
#include "Consts.h"
#ifdef EnableTelegram
#include "Global.h"

extern void sendTelegramMsg();
extern void telegramInit();
extern void handleTelegram();
extern bool isEnableTelegramd();
extern bool isTelegramInputOn();
extern void telegramMsgParse(String msg);
extern String returnListOfParams();
#endif