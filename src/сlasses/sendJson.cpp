#include "сlasses/sendJson.h"

//базовый класс

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
        send(jsonArrayElement, _num);
    }
}

SendJson* mySendJson;
