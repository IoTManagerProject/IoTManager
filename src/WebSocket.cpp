#include "WebSocket.h"

#include "Global.h"

void wsSendSetup() {
    File file = seekFile("/setup.json");
    DynamicJsonDocument doc(1024);
    file.find("[");
    do {
        deserializeJson(doc, file);
        wsPublishData(F("config"), doc.as<String>());
    } while (file.findUntil(",", "]"));
}

void wsPublishData(String topic, String data) {
    data = "[" + topic + "]" + data;
    ws.textAll(data);
}
