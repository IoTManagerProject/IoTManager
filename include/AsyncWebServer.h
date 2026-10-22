#pragma once
// =====================================================================================
// ASYNC_WEB_SERVER / ASYNC_WEB_SOCKETS: асинхронный веб-сервер и веб-сокеты
// -------------------------------------------------------------------------------------
// Реализация полностью поддерживает набор функций синхронного варианта
// (STANDARD_WEB_SERVER / STANDARD_WEB_SOCKETS):
//   * HTTP: статика (в том числе .gz), /set, /status, /list, /edit (просмотр, создание,
//     удаление, загрузка файлов), /localota, /localota_handler, /update (OTA), CORS;
//   * WebSocket: тот же протокол, что и у standWebSocket (порт 81, "/"),
//     заголовки сообщений "xxxxxx|0012|", фрагментация больших файлов по фреймам.
// =====================================================================================

#include "Global.h"

#ifdef ASYNC_WEB_SERVER
// общий код инициализации обработчиков HTTP (см. StandWebServer.cpp): используется и для
// STANDARD_WEB_SERVER, и для ASYNC_WEB_SERVER
void asyncWebServerInit();
#endif

#ifdef ASYNC_WEB_SOCKETS

// Сервер веб-сокетов. Веб-интерфейс (Svelte) подключается к ws://<ip>:81 без пути,
// поэтому асинхронные сокеты поднимаются на порту 81 (как standWebSocket) с url "/"
extern AsyncWebServer wsAsyncServer;
extern AsyncWebSocket ws;
extern AsyncEventSource events;  // [DEPRECATED] SSE не используется прошивкой, оставлено для совместимости

void asyncWebSocketsInit();
// обработка «тяжёлых» WS-команд (в том числе отправка файлов) вынесена в loop(), так как
// колбэк onWsEvent на ESP32 выполняется в задаче AsyncTCP и блокировать её нельзя
void asyncWebSocketsLoop();
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);

// ---- слоты клиентов -----------------------------------------------------------------
// Веб-интерфейс обращается к клиентам по номерам (num 0..WEBSOCKETS_CLIENT_MAX-1), как в
// WebSocketsServer. У клиентов AsyncWebSocket свои id (uint32_t), поэтому они
// раскладываются по слотам, и весь общий код работает с номерами слотов.
uint32_t asyncWebSocketSlotToId(uint8_t slot);
int8_t asyncWebSocketSlotById(uint32_t id);
int8_t asyncWebSocketSlotAdd(uint32_t id);
void asyncWebSocketSlotRemove(uint32_t id);
void asyncWebSocketSlotReset();

// ---- отправка данных клиентам (транспортный уровень, вызывается из WsServer.cpp) ----
void asyncWebSocketSendText(uint8_t slot, const String& msg);
void asyncWebSocketSendBin(uint8_t slot, uint8_t *data, size_t size, bool fin, bool continuation);
void asyncWebSocketSendBinAll(uint8_t *data, size_t size, bool fin, bool continuation);
void asyncWebSocketSendFileFrames(File &file, const String &header, const String &json, int client_id, size_t frameSize);
void asyncWebSocketDisconnect(uint8_t slot);
int asyncWebSocketCount();

#endif
