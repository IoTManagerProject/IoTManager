#include "Global.h"

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

CaptiveRequestHandler::CaptiveRequestHandler(const char *host) {
    strlcpy(_local_name, host, sizeof(_local_name));
    strcat(_local_name, ".local");
}

CaptiveRequestHandler::~CaptiveRequestHandler() {
}

bool CaptiveRequestHandler::isLocalIp(String address) {
    IPAddress ip;
    return !ip.fromString(address) || (ip != _local_ip);
}

bool CaptiveRequestHandler::isLocalName(String host_name) {
    return host_name.equalsIgnoreCase(_local_name);
}

bool CaptiveRequestHandler::canHandle(AsyncWebServerRequest *request) {
    _local_ip = request->client()->localIP();

    return !isLocalIp(request->getHeader("HOST")->value()) && !isLocalName(request->getHeader("HOST")->value());
}

void CaptiveRequestHandler::CaptiveRequestHandler::handleRequest(AsyncWebServerRequest *request) {
    char buf[64];
    sprintf(buf, "http://%s%s", _local_name, request->url().c_str());
    auto response = request->beginResponse(302, "text/html");
    response->addHeader("Location", buf);
    response->addHeader("Connection", "close");
    request->send(response);
};
