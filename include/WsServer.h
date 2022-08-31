#pragma once
#include "Global.h"
#include "utils/WiFiUtils.h"
#include "DeviceList.h"
#include "ESPConfiguration.h"
#include "UpgradeFirm.h"

#ifdef STANDARD_WEB_SOCKETS
extern void standWebSocketsInit();
extern void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
#ifdef ESP32
extern void hexdump(const void* mem, uint32_t len, uint8_t cols);
#endif
#endif

void sendFileToWs(const char* filename, uint8_t num, size_t frameSize);
void publishStatusWs(const String& topic, const String& data);
void publishStatusWsJson(String& json);
void periodicWsSend();
void sendStringToWs(const String& msg, uint8_t num, String name);

// void sendMark(const char* filename, const char* mark, uint8_t num);
// void sendFileToWs3(const String& filename, uint8_t num);
// void sendFileToWs4(const String& filename, uint8_t num);