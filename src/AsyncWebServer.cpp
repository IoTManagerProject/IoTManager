#include "AsyncWebServer.h"
#ifdef ASYNC_WEB_SERVER

void asyncWebServerInit() {
    String login = jsonReadStr(settingsFlashJson, "weblogin");
    String pass = jsonReadStr(settingsFlashJson, "webpass");
#ifdef ESP32
    // server.addHandler(new FSEditor(FileFS, login, pass));
#else
    // server.addHandler(new FSEditor(login, pass));
#endif

#ifdef CORS_DEBUG
    DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Origin"), F("*"));
    DefaultHeaders::Instance().addHeader(F("Access-Control-Allow-Headers"), F("content-type"));
#endif

    server.serveStatic("/css/", FileFS, "/css/").setCacheControl("max-age=600");
    server.serveStatic("/js/", FileFS, "/js/").setCacheControl("max-age=600");
    server.serveStatic("/favicon.ico", FileFS, "/favicon.ico").setCacheControl("max-age=600");
    server.serveStatic("/icon.jpeg", FileFS, "/icon.jpeg").setCacheControl("max-age=600");
    server.serveStatic("/edit", FileFS, "/edit").setCacheControl("max-age=600");

    server.serveStatic("/", FileFS, "/").setDefaultFile("index.html").setAuthentication(login.c_str(), pass.c_str());

    server.onNotFound([](AsyncWebServerRequest *request) {
        // SerialPrint("[E]", "WebServer", "not found:\n" + getRequestInfo(request));
        request->send(404);
    });

    server.onFileUpload([](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {
        // TODO
        if (!index) {
            // SerialPrint('I', "WebServer", "start upload " + filename);
        }
        if (final) {
            // SerialPrint('I', "WebServer", "finish upload: " + prettyBytes(index + len));
        }
    });

    //Обработка гет запросов

    // динамические данные
    // server.on("/config.live.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    //    request->send(200, "application/json", paramsFlashJson);
    //});
    //
    // server.on("/config.store.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    //    request->send(200, "application/json", paramsFlashJson);
    //});
    //
    // server.on("/config.option.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    //    request->send(200, "application/json", paramsHeapJson);
    //});
    //
    // server.on("/config.setup.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    //    request->send(200, "application/json", settingsFlashJson);
    //});
    //
    // server.on("/cmd", HTTP_GET, [](AsyncWebServerRequest *request) {
    //    String cmdStr = request->getParam("command")->value();
    //SerialPrint('I', "WebServer", "do: " + cmdStr);
    //    // loopCmdAdd(cmdStr);
    //    request->send(200, "text/html", "OK");
    //});

    server.begin();

SerialPrint('I', F("WEB"), F("WebServer Init"));
}
#endif

#ifdef ASYNC_WEB_SOCKETS

AsyncWebSocket ws("/ws");
AsyncEventSource events("/events");

void asyncWebSocketsInit() {
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    events.onConnect([](AsyncEventSourceClient *client) {
        client->send("", NULL, millis(), 1000);
    });
    server.addHandler(&events);
SerialPrint('I', F("WEB"), F("WebSockets Init"));
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
        client->printf("Hello Client %u :)", client->id());
        client->ping();
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
    } else if (type == WS_EVT_ERROR) {
        Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
    } else if (type == WS_EVT_PONG) {
        Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
    } else if (type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        String msg = "";
        if (info->final && info->index == 0 && info->len == len) {
            // the whole message is in a single frame and we got all of it's data
            Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);

            if (info->opcode == WS_TEXT) {
                for (size_t i = 0; i < info->len; i++) {
                    msg += (char)data[i];
                }
            } else {
                char buff[3];
                for (size_t i = 0; i < info->len; i++) {
                    sprintf(buff, "%02x ", (uint8_t)data[i]);
                    msg += buff;
                }
            }
            Serial.printf("%s\n", msg.c_str());

            if (info->opcode == WS_TEXT)
                client->text("I got your text message");
            else
                client->binary("I got your binary message");
        } else {
            // message is comprised of multiple frames or the frame is split into multiple packets
            if (info->index == 0) {
                if (info->num == 0)
                    Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
                Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
            }

            Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT) ? "text" : "binary", info->index, info->index + len);

            if (info->opcode == WS_TEXT) {
                for (size_t i = 0; i < len; i++) {
                    msg += (char)data[i];
                }
            } else {
                char buff[3];
                for (size_t i = 0; i < len; i++) {
                    sprintf(buff, "%02x ", (uint8_t)data[i]);
                    msg += buff;
                }
            }
            Serial.printf("%s\n", msg.c_str());

            if ((info->index + len) == info->len) {
                Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
                if (info->final) {
                    Serial.printf("ws[%s][%u] %s-message end\n", server->url(), client->id(), (info->message_opcode == WS_TEXT) ? "text" : "binary");
                    if (info->message_opcode == WS_TEXT)
                        client->text("I got your text message");
                    else
                        client->binary("I got your binary message");
                }
            }
        }
    }
}
#endif
