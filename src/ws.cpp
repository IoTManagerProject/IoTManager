#include "ws.h"

WebSockets::WebSockets() {}

WebSockets::~WebSockets() {}

void WebSockets::sendFile(String path) {
    _path = path;
    file = seekFile(path);
    // file.find("[");
}

// void WebSockets::loop() {
//     if (ws.availableForWriteAll()) {
//         if (file.findUntil(",", "]")) {
//             DynamicJsonDocument doc(1024);
//             deserializeJson(doc, file);
//             // Serial.println(doc.as<String>());
//             ws.textAll("[" + _path + "]" + doc.as<String>());
//         }
//     }
// }

void WebSockets::loop() {
    if (ws.availableForWriteAll()) {
        if (file.available()) {
            String jsonArrayElement = file.readStringUntil('}') + "}";
            Serial.println(jsonArrayElement);
            ws.textAll(jsonArrayElement);
        }
    }
}

WebSockets* myWebSockets;
