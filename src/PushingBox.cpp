#include "Global.h"

static const char* MODULE = "pushingbox";

static const char* logServer = "api.pushingbox.com";

void Push_init() {
    server.on("/pushingboxDate", HTTP_GET, [](AsyncWebServerRequest* request) {
        if (request->hasArg("pushingbox_id")) {
            config.general()->setPushingboxId(request->getParam("pushingbox_id")->value());
        }
        request->send(200);
    });
}

void pushControl(const String& title, const String& body) {
    WiFiClient wifiClient;
    pm.info("connecting: " + String(logServer));

    if (!wifiClient.connect(logServer, 80)) {
        pm.error("failed");
        return;
    }

    String payload = "devid=";
    payload += String(config.general()->getPushingboxId());
    payload += "&title=";
    payload += title;
    payload += "&body=";
    payload += body;
    payload += "\r\n\r\n";

    wifiClient.print("POST /pushingbox HTTP/1.1\n");
    wifiClient.print("Host: api.pushingbox.com\n");
    wifiClient.print("Connection: close\n");
    wifiClient.print("Content-Type: application/x-www-form-urlencoded\n");
    wifiClient.print("Content-Length: ");
    wifiClient.print(payload.length());
    wifiClient.print("\n\n");
    wifiClient.print(payload);

    wifiClient.stop();
}
