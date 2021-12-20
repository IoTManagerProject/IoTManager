#include "WebSocket.h"

#include <ArduinoJson.h>

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
    File file = seekFile("/setup.json");

    DynamicJsonDocument doc(2048);

    int i = 0;

    file.find("[");

    SerialPrint("I", F("WS"), F("start send config"));
    do {
        i++;

        // static bool flag = false;
        deserializeJson(doc, file);
        size_t len = measureJson(doc);
        AsyncWebSocketMessageBuffer* buffer = ws.makeBuffer(len);
        if (buffer) {
            serializeJson(doc, (char*)buffer->get(), len);
            if (ws.enabled()) {
                while (!ws.availableForWriteAll()) {
                    Serial.println(String(i) + ") not ready");
                }
                ws.textAll(buffer);
                Serial.println(String(i) + ") ready");

                // if (ws.availableForWriteAll()) {
                //     ws.textAll(buffer);
                //     Serial.println(String(i) + ") ready");
                // } else {
                //     Serial.println(String(i) + ") not ready");
                //     delay(100);
                // }
            }
        }
        //Serial.println(doc.as<String>());

    } while (file.findUntil(",", "]"));

    SerialPrint("I", F("WS"), F("completed send config"));
}

void wsPublishData(String topic, String data) {
    if (ws.enabled()) {
        data = "[" + topic + "]" + data;
        ws.textAll(data);
    }
}

// wsPublishData(F("config"), doc.as<String>());
// if (ws.enabled()) {
//}
// if (ws.enabled()) Serial.println("on");

// void sendDataWs() {
//     DynamicJsonDocument doc(1024);
//
//     doc["a"] = "abc";
//     doc["b"] = "abcd";
//     doc["c"] = "abcde";
//     doc["d"] = "abcdef";
//     doc["e"] = "abcdefg";
//     size_t len = measureJson(doc);
//     AsyncWebSocketMessageBuffer* buffer = ws.makeBuffer(len);  //  creates a buffer (len + 1) for you.
//     if (buffer) {
//         serializeJson(doc, (char*)buffer->get(), len);
//         ws.textAll(buffer);
//     }
// }
