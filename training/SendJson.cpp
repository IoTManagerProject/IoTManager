#include "classes/SendJson.h"
#ifdef QUEUE_FROM_STR

SendJson::SendJson() {
    filesQueue = new QueueFromStruct;
}
SendJson::~SendJson() {}

void SendJson::addFileToQueue(String path, uint8_t num) {
    myItem.myword = path;
    myItem.num = num;
    filesQueue->push(myItem);
    SerialPrint(F("i"), F("WS"), "file added to Queue " + path);
}

//опсылает массив json по объектно в сокеты
void SendJson::loop() {
    if (!filesQueue->empty() && !sendingInProgress) {
        Serial.println("Queue not empty");
        myItem = filesQueue->front();
        _path = myItem.myword;
        _num = myItem.num;
        file = seekFile(_path);
        SerialPrint(F("i"), F("WS"), "seek File to WS " + _path);
        sendingInProgress = true;
    }
    if (file.available()) {
        String jsonArrayElement = _path + file.readStringUntil('}') + "}";
        sendWs(jsonArrayElement);
    } else {
        sendingInProgress = false;
    }
}

void SendJson::sendWs(String& jsonArrayElement) {
    standWebSocket.sendTXT(_num, jsonArrayElement);
}

void SendJson::sendMqtt(String& jsonArrayElement) {
    // mqtt send to do
}

SendJson* sendJsonFiles;
#endif