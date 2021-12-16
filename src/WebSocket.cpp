#include "WebSocket.h"

#include "Global.h"

void wsSendSetup() {
    SerialPrint("I", F("WS"), F("start send config"));
    File file = seekFile("/setup.json");

    DynamicJsonDocument doc(2048);
    // AsyncWebSocketMessageBuffer(20480);

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

// void sendDataWs(AsyncWebSocketClient* client) {
//     // DynamicJsonBuffer jsonBuffer;
//     // JsonObject& root = jsonBuffer.createObject();
//     // root["a"] = "abc";
//     // root["b"] = "abcd";
//     // root["c"] = "abcde";
//     // root["d"] = "abcdef";
//     // root["e"] = "abcdefg";
//     // size_t len = root.measureLength();
//     AsyncWebSocketMessageBuffer* buffer = ws.makeBuffer(len);  //  creates a buffer (len + 1) for you.
//     if (buffer) {
//         root.printTo((char*)buffer->get(), len + 1);
//         if (client) {
//             client->text(buffer);
//         } else {
//             ws.textAll(buffer);
//         }
//     }
// }
