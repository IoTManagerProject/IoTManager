#include "WsServer.h"
#ifdef STANDARD_WEB_SOCKETS
void standWebSocketsInit() {
    standWebSocket.begin();
    standWebSocket.onEvent(webSocketEvent);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_ERROR: {
            Serial.printf("[%u] Error!\n", num);
        } break;

        case WStype_DISCONNECTED: {
            Serial.printf("[%u] Disconnected!\n", num);
        } break;

        case WStype_CONNECTED: {
            IPAddress ip = standWebSocket.remoteIP(num);
            Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
            standWebSocket.sendTXT(num, "Connected");
        } break;

        case WStype_TEXT: {
            //максимальное количество символов заголовка
            size_t headerLenth = 7;
            String headerStr;
            headerStr.reserve(headerLenth + 1);
            for (size_t i = 0; i < headerLenth; i++) {
                headerStr += (char)payload[i];
            }
            // dashboard===================================================================
            if (headerStr == "/") {
                sendFileToWs5("/layout.json", num, 1024);
            }
            if (headerStr == "/tuoyal") {
                writeFileUint8tByFrames("layout.json", payload, length, headerLenth, 256);
            }
            // configutation===============================================================
            if (headerStr == "/config") {
                sendFileToWs5("/items.json", num, 1024);
                sendFileToWs5("/widgets.json", num, 1024);
                sendFileToWs5("/config.json", num, 1024);
            }
            if (headerStr == "/gifnoc") {
                writeFileUint8tByFrames("config.json", payload, length, headerLenth, 256);
            }
            // connection===================================================================
            if (headerStr == "/connec") {
                sendFileToWs5("/settings.json", num, 1024);
            }
            if (headerStr == "/cennoc") {
                writeFileUint8tByFrames("settings.json", payload, length, headerLenth, 256);
            }

        } break;

        case WStype_BIN: {
            Serial.printf("[%u] get binary length: %u\n", num, length);
            // hexdump(payload, length);
            // standWebSocket.sendBIN(num, payload, length);
        } break;

        case WStype_FRAGMENT_TEXT_START: {
            Serial.printf("[%u] fragment test start: %u\n", num, length);
        } break;

        case WStype_FRAGMENT_BIN_START: {
            Serial.printf("[%u] fragment bin start: %u\n", num, length);
        } break;

        case WStype_FRAGMENT: {
            Serial.printf("[%u] fragment: %u\n", num, length);
        } break;

        case WStype_FRAGMENT_FIN: {
            Serial.printf("[%u] fragment finish: %u\n", num, length);
        } break;

        case WStype_PING: {
            Serial.printf("[%u] ping: %u\n", num, length);
        } break;

        case WStype_PONG: {
            Serial.printf("[%u] pong: %u\n", num, length);
        } break;

        default: {
            Serial.printf("[%u] not recognized: %u\n", num, length);
        } break;
    }
}

#ifdef ESP32
void hexdump(const void* mem, uint32_t len, uint8_t cols = 16) {
    const uint8_t* src = (const uint8_t*)mem;
    Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
    for (uint32_t i = 0; i < len; i++) {
        if (i % cols == 0) {
            Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
        }
        Serial.printf("%02X ", *src);
        src++;
    }
    Serial.printf("\n");
}
#endif
#endif

//посылка данных из файла в бинарном виде
void sendFileToWs5(const char* filename, uint8_t num, size_t frameSize) {
    standWebSocket.sendTXT(num, "/st" + String(filename));
    String path = filepath(filename);
    auto file = FileFS.open(path, "r");
    if (!file) {
        SerialPrint(F("E"), F("FS"), F("reed file error"));
        return;
    }
    size_t fileSize = file.size();
    SerialPrint(F("i"), F("FS"), "Send file '" + String(filename) + "', file size: " + String(fileSize));
    uint8_t payload[frameSize];
    int countRead = file.read(payload, sizeof(payload));
    while (countRead > 0) {
        standWebSocket.sendBIN(num, payload, countRead);
        countRead = file.read(payload, sizeof(payload));
    }
    file.close();
    standWebSocket.sendTXT(num, "/end" + String(filename));
}

void publishStatusWs(const String& topic, const String& data) {
    String path = mqttRootDevice + "/" + topic + "/status";
    String json = "{}";
    jsonWriteStr(json, "status", data);
    jsonWriteStr(json, "topic", path);
    standWebSocket.broadcastTXT(json);
}

// void sendMark(const char* filename, const char* mark, uint8_t num) {
//     char outChar[strlen(filename) + strlen(mark) + 1];
//     strcpy(outChar, mark);
//     strcat(outChar, filename);
//     size_t size = strlen(outChar);
//     uint8_t outUint[size];
//     for (size_t i = 0; i < size; i++) {
//         outUint[i] = uint8_t(outChar[i]);
//     }
//     standWebSocket.sendBIN(num, outUint, sizeof(outUint));
// }

//посылка данных из файла в string
// void sendFileToWs3(const String& filename, uint8_t num) {
//    standWebSocket.sendTXT(num, "/st" + filename);
//    size_t ws_buffer = 512;
//    String path = filepath(filename);
//    auto file = FileFS.open(path, "r");
//    if (!file) {
//        SerialPrint(F("E"), F("FS"), F("reed file error"));
//    }
//    size_t fileSize = file.size();
//    SerialPrint(F("i"), F("WS"), "Send file '" + filename + "', file size: " + String(fileSize));
//    String ret;
//    char temp[ws_buffer + 1];
//    int countRead = file.readBytes(temp, sizeof(temp) - 1);
//    while (countRead > 0) {
//        temp[countRead] = 0;
//        ret = temp;
//        standWebSocket.sendTXT(num, ret);
//        countRead = file.readBytes(temp, sizeof(temp) - 1);
//    }
//    standWebSocket.sendTXT(num, "/end" + filename);
//}

//посылка данных из файла в char
// void sendFileToWs4(const String& filename, uint8_t num) {
//    standWebSocket.sendTXT(num, "/st" + filename);
//    size_t ws_buffer = 512;
//    String path = filepath(filename);
//    auto file = FileFS.open(path, "r");
//    if (!file) {
//        SerialPrint(F("E"), F("FS"), F("reed file error"));
//    }
//    size_t fileSize = file.size();
//    SerialPrint(F("i"), F("WS"), "Send file '" + filename + "', file size: " + String(fileSize));
//    char temp[ws_buffer + 1];
//    int countRead = file.readBytes(temp, sizeof(temp) - 1);
//    while (countRead > 0) {
//        temp[countRead] = 0;
//        standWebSocket.sendTXT(num, temp, countRead);
//        countRead = file.readBytes(temp, sizeof(temp) - 1);
//    }
//}
