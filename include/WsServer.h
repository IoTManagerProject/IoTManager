#pragma once
#include "Global.h"
#include "utils/WiFiUtils.h"
#include "DeviceList.h"
#include "ESPConfiguration.h"
#include "UpgradeFirm.h"

// Оба варианта веб-сокетов должны быть включены (проверяется в Const.h):
//  * STANDARD_WEB_SOCKETS — библиотека WebSocketsServer (standWebSocket);
//  * ASYNC_WEB_SOCKETS   — AsyncWebSocket (ws, порт 81, см. AsyncWebServer.h).
#if defined(STANDARD_WEB_SOCKETS) || defined(ASYNC_WEB_SOCKETS)

#ifdef STANDARD_WEB_SOCKETS
extern void standWebSocketsInit();
extern void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
#ifdef ESP32
extern void hexdump(const void* mem, uint32_t len, uint8_t cols);
#endif
#else
extern void asyncWebSocketsInit();
extern void asyncWebSocketsLoop();   // периодическое обслуживание входящих WS-сообщений (вызывается из Main::loop)
#endif

// ---- транспорт веб-сокетов: единый интерфейс для обоих вариантов ----
// (реализация выбирается по define в WsServer.cpp, кода обработчиков не касается)
void webSocketSendText(uint8_t num, const String& msg);
void webSocketSendBin(uint8_t num, uint8_t* data, size_t size, bool fin, bool continuation);
void webSocketSendBinAll(uint8_t* data, size_t size, bool fin, bool continuation);
void webSocketDisconnectClient(uint8_t num);
int webSocketConnectedClients();

// обработка текстовых команд веб-интерфейса — общая для STANDARD_WEB_SOCKETS и ASYNC_WEB_SOCKETS
void handleWsTextMessage(uint8_t num, uint8_t* payload, size_t length);
#endif

void publishStatusWs(const String& topic, const String& data);
void publishJsonWs(const String& topic, String& json);
void periodicWsSend();

void sendFileToWsByFrames(const String& filename, const String& header, const String& json, int client_id, size_t frameSize);
void sendStringToWs(const String& header, String& payload, int client_id);
void disconnectWSClient(uint8_t client_id);
void sendDeviceList(uint8_t num);
int getNumWSClients();
