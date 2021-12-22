#pragma once

#include "Global.h"

extern AsyncWebSocket ws;
extern AsyncEventSource events;

void webServerInit();
void webSocketsInit();
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
