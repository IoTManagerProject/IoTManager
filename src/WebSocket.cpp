#include "WebSocket.h"

#include "ArduinoJson.h"
#include "Class/CircularBuffer.h"
#include "Class/NotAsync.h"
#include "Global.h"

void wsInit() {
    // myNotAsyncActions->add(
    //     do_webSocketSendSetup, [&](void*) {
    //         delay(100);
    //         wsSendSetup();
    //     },
    //     nullptr);
}

void wsPublishData(String topic, String data) {
    if (ws.enabled()) {
        // if (ws.availableForWriteAll()) {
        data = "[" + topic + "]" + data;
        ws.textAll(data);
        //}
    }
}

//отправка setup массива в sockets способом через буфер string, рабочий способ но буфер стринг - плохой метод
void wsSendSetup() {
    File file = seekFile("/setup.json");
    DynamicJsonDocument doc(2048);
    int i = 0;
    file.find("[");
    SerialPrint("I", F("WS"), F("start send config"));
    do {
        i++;
        deserializeJson(doc, file);
        wsBuf += doc.as<String>() + "\n";
    } while (file.findUntil(",", "]"));
    SerialPrint("I", F("WS"), F("completed send config"));
}

void loopWsExecute() {
    static int attampts = wsAttempts;
    if (wsBuf.length()) {
        if (attampts > 0) {
            if (ws.availableForWriteAll()) {
                String tmp = selectToMarker(wsBuf, "\n");
                wsPublishData("config", tmp);
                wsBuf = deleteBeforeDelimiter(wsBuf, "\n");
                attampts = wsAttempts;
            } else {
                attampts--;
                SerialPrint("I", F("WS"), String(attampts));
            }
        } else {
            SerialPrint("I", F("WS"), F("socket fatal error"));
            attampts = wsAttempts;
        }
    }
}

//отправка setup массива в sockets способом через кольщевой буфер char
void wsSendSetup2() {
    // myWsBuffer = new CircularBuffer<char*, 20480>;
    // File file = seekFile("/setup.json");
    // DynamicJsonDocument doc(2048);
    // int i = 0;
    // file.find("[");
    // SerialPrint("I", F("WS"), F("start send config"));
    // do {
    //     i++;
    //     deserializeJson(doc, file);
    //     char* element = "";
    //     serializeJson(doc, (char*)element, 1024);
    //     Serial.println(element);
    //     myWsBuffer->push(element);
    //
    //} while (file.findUntil(",", "]"));
    // SerialPrint("I", F("WS"), F("completed send config"));
}

void loopWsExecute2() {
    // char* item;
    // if (myWsBuffer->pop(item)) {
    //     Serial.println(item);
    // }
}

//отправка setup массива в sockets способом прямой загрузки в ws buffer
void wsSendSetupBuffer() {
    File file = seekFile("/setup.json");
    DynamicJsonDocument doc(2048);
    int i = 0;
    file.find("[");
    SerialPrint("I", F("WS"), F("start send config"));
    do {
        i++;
        deserializeJson(doc, file);
        size_t len = measureJson(doc);
        AsyncWebSocketMessageBuffer* buffer = ws.makeBuffer(len);
        if (buffer) {
            serializeJson(doc, (char*)buffer->get(), len);
            if (ws.enabled()) {
                if (ws.availableForWriteAll()) {
                    ws.textAll(buffer);
                }
            }
        }
        // Serial.println(doc.as<String>());
    } while (file.findUntil(",", "]"));
    SerialPrint("I", F("WS"), F("completed send config"));
}

//Пример прямого доступа к web socket buffer переделанный для arduino json 6
void sendDataWs() {
    DynamicJsonDocument doc(1024);
    doc["a"] = "abc";
    doc["b"] = "abcd";
    doc["c"] = "abcde";
    doc["d"] = "abcdef";
    doc["e"] = "abcdefg";
    size_t len = measureJson(doc);
    AsyncWebSocketMessageBuffer* buffer = ws.makeBuffer(len);
    if (buffer) {
        serializeJson(doc, (char*)buffer->get(), len);
        ws.textAll(buffer);
    }
}
