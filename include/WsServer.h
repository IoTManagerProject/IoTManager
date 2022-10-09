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

void sendFileToWs(String filename, int num, size_t frameSize);
void publishStatusWs(const String& topic, const String& data);
void publishChartWs(int num, String& path);
void periodicWsSend();

void publishChartToWs(String filename, int num, size_t frameSize, int maxCount, String id);

void sendFileToWsByFrames(const String& filename, const String& header, const String& json, uint8_t client_id, size_t frameSize);
void sendStringToWs(const String& header, String& payload, int client_id);

// void sendMark(const char* filename, const char* mark, uint8_t num);
// void sendFileToWs3(const String& filename, uint8_t num);
// void sendFileToWs4(const String& filename, uint8_t num);