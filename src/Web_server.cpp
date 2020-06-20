#include "Global.h"

void Web_server_init() {
    /*********************************************************************************
  ***************************************OTA****************************************
  *********************************************************************************/
#ifdef OTA_enable
    ArduinoOTA.onStart([]() {
        events.send("Update Start", "ota");
    });
    ArduinoOTA.onEnd([]() {
        events.send("Update End", "ota");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        char p[32];
        sprintf(p, "Progress: %u%%\n", (progress / (total / 100)));
        events.send(p, "ota");
    });
    ArduinoOTA.onError([](ota_error_t error) {
        if (error == OTA_AUTH_ERROR)
            events.send("Auth Failed", "ota");
        else if (error == OTA_BEGIN_ERROR)
            events.send("Begin Failed", "ota");
        else if (error == OTA_CONNECT_ERROR)
            events.send("Connect Failed", "ota");
        else if (error == OTA_RECEIVE_ERROR)
            events.send("Recieve Failed", "ota");
        else if (error == OTA_END_ERROR)
            events.send("End Failed", "ota");
    });
    ArduinoOTA.setHostname(hostName);
    ArduinoOTA.begin();
#endif
    /*********************************************************************************
  **************************************MDNS****************************************
  *********************************************************************************/
#ifdef MDNS_enable
    MDNS.addService("http", "tcp", 80);
#endif
    //LittleFS.begin();
    /*********************************************************************************
  **************************************WS******************************************
  *********************************************************************************/
#ifdef WS_enable
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);

    events.onConnect([](AsyncEventSourceClient *client) {
        //!!!client->send("hello!", NULL, millis(), 1000);
    });

    server.addHandler(&events);
#endif
    /*********************************************************************************
   **************************************WEB****************************************
   *********************************************************************************/
#ifdef ESP32
    server.addHandler(new SPIFFSEditor(LittleFS, jsonReadStr(configSetupJson, "weblogin").c_str(), jsonReadStr(configSetupJson, "webpass").c_str()));
#elif defined(ESP8266)
    server.addHandler(new SPIFFSEditor(jsonReadStr(configSetupJson, "weblogin").c_str(), jsonReadStr(configSetupJson, "webpass").c_str()));
#endif

    /*  server.on("/heap", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(200, "text/plain", String(ESP.getFreeHeap()));
    });*/

    server.serveStatic("/css/", LittleFS, "/css/").setCacheControl("max-age=31536000");
    server.serveStatic("/js/", LittleFS, "/js/").setCacheControl("max-age=31536000");
    server.serveStatic("/favicon.ico", LittleFS, "/favicon.ico").setCacheControl("max-age=31536000");
    server.serveStatic("/icon.jpeg", LittleFS, "/icon.jpeg").setCacheControl("max-age=31536000");

    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.htm").setAuthentication(jsonReadStr(configSetupJson, "weblogin").c_str(), jsonReadStr(configSetupJson, "webpass").c_str());

    server.onNotFound([](AsyncWebServerRequest *request) {
        Serial.printf("NOT_FOUND: ");
        if (request->method() == HTTP_GET)
            Serial.printf("GET");
        else if (request->method() == HTTP_POST)
            Serial.printf("POST");
        else if (request->method() == HTTP_DELETE)
            Serial.printf("DELETE");
        else if (request->method() == HTTP_PUT)
            Serial.printf("PUT");
        else if (request->method() == HTTP_PATCH)
            Serial.printf("PATCH");
        else if (request->method() == HTTP_HEAD)
            Serial.printf("HEAD");
        else if (request->method() == HTTP_OPTIONS)
            Serial.printf("OPTIONS");
        else
            Serial.printf("UNKNOWN");
        Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

        if (request->contentLength()) {
            Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
            Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
        }

        int headers = request->headers();
        int i;
        for (i = 0; i < headers; i++) {
            AsyncWebHeader *h = request->getHeader(i);
            Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
        }

        int params = request->params();
        for (i = 0; i < params; i++) {
            AsyncWebParameter *p = request->getParam(i);
            if (p->isFile()) {
                Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
            } else if (p->isPost()) {
                Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
            } else {
                Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
            }
        }

        request->send(404);
    });

    server.onFileUpload([](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {
        if (!index)
            Serial.printf("UploadStart: %s\n", filename.c_str());
        Serial.printf("%s", (const char *)data);
        if (final)
            Serial.printf("UploadEnd: %s (%u)\n", filename.c_str(), index + len);
    });

    server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        if (!index)
            Serial.printf("BodyStart: %u\n", total);
        Serial.printf("%s", (const char *)data);
        if (index + len == total)
            Serial.printf("BodyEnd: %u\n", total);
    });

    server.begin();

    // --------------------Выдаем данные configJson //config.live.json - динамические данные
    server.on("/config.live.json", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", configLiveJson);
    });
    // --------------------Выдаем данные optionJson //config.option.json - данные не являющиеся событиями
    server.on("/config.option.json", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", configOptionJson);
    });
    // -------------------Выдаем данные configSetup //config.setup.json - для хранения постоянных данных
    server.on("/config.setup.json", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", configSetupJson);
    });

    // ------------------Выполнение команды из запроса
    //http://192.168.88.45/cmd?command=rel%201%201
    server.on("/cmd", HTTP_GET, [](AsyncWebServerRequest *request) {
        String com = request->getParam("command")->value();
        Serial.println(com);
        order_loop += com + ",";
        request->send(200, "text/text", "OK");  // отправляем ответ о выполнении
    });
}
/*********************************************************************************************************************************
 *********************************************************WS**********************************************************************
 ********************************************************************************************************************************/
#ifdef WS_enable
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
        client->printf(json.c_str(), client->id());
        //client->ping();
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
            //the whole message is in a single frame and we got all of it's data
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
                client->text("{}");
            else
                client->binary("{}");
        } else {
            //message is comprised of multiple frames or the frame is split into multiple packets
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