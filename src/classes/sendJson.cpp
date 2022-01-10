#include "classes/sendJson.h"

SendJson::SendJson() {}
SendJson::~SendJson() {}

void SendJson::sendFile(String path, uint8_t num) {
    _path = path;
    _num = num;
    file = seekFile(path);
}

void SendJson::loop() {
    if (file.available()) {
        String jsonArrayElement = _path + file.readStringUntil('}') + "}";
        sendWs(jsonArrayElement);
    }
}

void SendJson::sendWs(String& jsonArrayElement) {
    standWebSocket.sendTXT(_num, jsonArrayElement);
}

void SendJson::sendMqtt(String& jsonArrayElement) {
    // mqtt send to do
}

SendJson* mySendJson;
