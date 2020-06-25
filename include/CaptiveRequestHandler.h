#pragma once

#include <AsyncWebSocket.h>

class CaptiveRequestHandler : public AsyncWebHandler {
   public:
    CaptiveRequestHandler(const char *host);

    virtual ~CaptiveRequestHandler();

    bool canHandle(AsyncWebServerRequest *request) override;

    void handleRequest(AsyncWebServerRequest *request) override;

   private:
    bool isLocalIp(String name);
    bool isLocalName(String name);

   private:
    char _local_name[32];
    IPAddress _local_ip;
};
