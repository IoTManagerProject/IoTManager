#include "Web.h"

StreamJsonArray::StreamJsonArray() {}

StreamJsonArray::~StreamJsonArray() {}

void StreamJsonArray::sendFile(String path, uint8_t num) {
    _path = path;
    _num = num;
    file = seekFile(path);
    // file.find("[");
}

void StreamJsonArray::loop() {
    // if (ws.availableForWriteAll()) {
    if (file.available()) {
        String jsonArrayElement = file.readStringUntil('}') + "}";
        // Serial.println(jsonArrayElement);
        standWebSocket.sendTXT(_num, jsonArrayElement);
        // ws.textAll(jsonArrayElement);
    }
    //}
}

StreamJsonArray* myStreamJsonArray;