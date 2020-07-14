#include "PushingBox.h"

#include "Global.h"

namespace PushingBox {

static const char* MODULE = "pushingbox";
static const char* logServer = "api.pushingbox.com";

void init() {
    server.on("/pushingboxDate", HTTP_GET, [](AsyncWebServerRequest* request) {
        if (request->hasArg("pushingbox_id")) {
            config.general()->setPushingboxId(request->getParam("pushingbox_id")->value());
        }
        request->send(200);
    });
}

void post(const String& title, const String& body) {
    WiFiClient wifiClient;
    pm.info("connecting: " + String(logServer));

    if (!wifiClient.connect(logServer, 80)) {
        pm.error("failed");
        return;
    }

    String payload = "devid=";
    payload += config.general()->getPushingboxId();
    payload += "&title=";
    payload += title;
    payload += "&body=";
    payload += body;
    payload += "\r\n\r\n";

    wifiClient.print(F("POST /pushingbox HTTP/1.1\n"));
    wifiClient.print("Host: " + String(logServer) + "\n");
    wifiClient.print(F("Connection: close\n"));
    wifiClient.print(F("Content-Type: application/x-www-form-urlencoded\n"));
    wifiClient.print(F("Content-Length: "));
    wifiClient.print(payload.length());
    wifiClient.print("\n\n");
    wifiClient.print(payload);

    wifiClient.stop();
}

}  // namespace PushingBox