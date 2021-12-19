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

        ws.textAll("[config]" + doc.as<char>());

        Serial.println(doc.as<String>());
    } while (file.findUntil(",", "]"));
    SerialPrint("I", F("WS"), F("completed send config"));
}

void wsPublishData(String topic, String data) {
    data = "[" + topic + "]" + data;
    ws.textAll(data);
}

// wsPublishData(F("config"), doc.as<String>());
// if (ws.enabled()) {
//}
// if (ws.enabled()) Serial.println("on");
