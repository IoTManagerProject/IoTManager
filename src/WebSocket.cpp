#include "WebSocket.h"

#include "Class/NotAsync.h"
#include "Global.h"

void wsInit() {
    myNotAsyncActions->add(
        do_webSocketSendSetup, [&](void*) {
            wsSendSetup();
        },
        nullptr);
}

void wsSendSetup() {
    SerialPrint("I", F("WS"), F("start send config"));
    File file = seekFile("/setup.json");

    DynamicJsonDocument doc(2048);

    file.find("[");
    do {
        deserializeJson(doc, file);

        // wsPublishData(F("config"), doc.as<String>());
        // if (ws.enabled()) {
        ws.textAll("[config]" + doc.as<String>());
        //}

        // if (ws.enabled()) Serial.println("on");

        Serial.println(doc.as<String>());

    } while (file.findUntil(",", "]"));
    SerialPrint("I", F("WS"), F("completed send config"));
}

void wsPublishData(String topic, String data) {
    data = "[" + topic + "]" + data;
    ws.textAll(data);
}
