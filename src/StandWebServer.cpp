#include "StandWebServer.h"
#ifdef STANDARD_WEB_SERVER

File fsUploadFile;

void standWebServerInit() {
    //  Кэшировать файлы для быстрой работы
    HTTP.serveStatic("/css/", FileFS, "/css/", "max-age=31536000");  // кеширование на 1 год
    HTTP.serveStatic("/js/", FileFS, "/js/", "max-age=31536000");    // кеширование на 1 год
    HTTP.serveStatic("/png/", FileFS, "/png/", "max-age=31536000");  // кеширование на 1 год

    HTTP.on("/settings.json", HTTP_GET, []() {
        HTTP.send(200, "application/json", settingsFlashJson);
    });

    HTTP.on("/config.json", HTTP_GET, []() {
        HTTP.send(200, "application/json", readFile(F("config.json"), 20000));
    });

    HTTP.on("/layout.json", HTTP_GET, []() {
        HTTP.send(200, "application/json", readFile(F("layout.json"), 20000));
    });

    HTTP.on("/restart", HTTP_GET, []() {
        // ESP.restart();
        HTTP.send(200, "text/plain", "ok");
    });

    //  Добавляем функцию Update для перезаписи прошивки по WiFi при 1М(256K FileFS) и выше
    //  httpUpdater.setup(&HTTP);
    //  Запускаем HTTP сервер
    HTTP.begin();

#ifdef REST_FILE_OPERATIONS
    SPIFFS.begin();
    {
        Dir dir = SPIFFS.openDir("/");
        while (dir.next()) {
            String fileName = dir.fileName();
            size_t fileSize = dir.fileSize();
        }
    }
    // HTTP страницы для работы с FFS
    // list directory
    HTTP.on("/list", HTTP_GET, handleFileList);
    //загрузка редактора editor
    HTTP.on("/edit", HTTP_GET, []() {
        if (!handleFileRead("/edit.htm")) HTTP.send(404, "text/plain", "FileNotFound");
    });
    //Создание файла
    HTTP.on("/edit", HTTP_PUT, handleFileCreate);
    //Удаление файла
    HTTP.on("/edit", HTTP_DELETE, handleFileDelete);
    // first callback is called after the request has ended with all parsed arguments
    // second callback handles file uploads at that location
    HTTP.on(
        "/edit", HTTP_POST, []() {
            HTTP.send(200, "text/plain", "");
        },
        handleFileUpload);
#endif
    // called when the url is not defined here
    // use it to load content from SPIFFS
    HTTP.onNotFound([]() {
        if (!handleFileRead(HTTP.uri()))
            HTTP.send(404, "text/plain", "FileNotFound");
    });
}

bool handleFileRead(String path) {
    if (path.endsWith("/")) path += "index.html";
    String contentType = getContentType(path);
    String pathWithGz = path + ".gz";
    if (FileFS.exists(pathWithGz) || FileFS.exists(path)) {
        if (FileFS.exists(pathWithGz))
            path += ".gz";
        File file = FileFS.open(path, "r");
        HTTP.streamFile(file, contentType);
        file.close();
        return true;
    }
    return false;
}

String getContentType(String filename) {
    if (HTTP.hasArg("download"))
        return "application/octet-stream";
    else if (filename.endsWith(".htm"))
        return "text/html";
    else if (filename.endsWith(".html"))
        return "text/html";
    else if (filename.endsWith(".json"))
        return "application/json";
    else if (filename.endsWith(".css"))
        return "text/css";
    else if (filename.endsWith(".js"))
        return "application/javascript";
    else if (filename.endsWith(".png"))
        return "image/png";
    else if (filename.endsWith(".gif"))
        return "image/gif";
    else if (filename.endsWith(".jpg"))
        return "image/jpeg";
    else if (filename.endsWith(".ico"))
        return "image/x-icon";
    else if (filename.endsWith(".xml"))
        return "text/xml";
    else if (filename.endsWith(".pdf"))
        return "application/x-pdf";
    else if (filename.endsWith(".zip"))
        return "application/x-zip";
    else if (filename.endsWith(".gz"))
        return "application/x-gzip";
    return "text/plain";
}

#ifdef REST_FILE_OPERATIONS
// Здесь функции для работы с файловой системой
void handleFileUpload() {
    if (HTTP.uri() != "/edit") return;
    HTTPUpload& upload = HTTP.upload();
    if (upload.status == UPLOAD_FILE_START) {
        String filename = upload.filename;
        if (!filename.startsWith("/")) filename = "/" + filename;
        fsUploadFile = FileFS.open(filename, "w");
        filename = String();
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        // Serial.print("handleFileUpload Data: "); Serial.println(upload.currentSize);
        if (fsUploadFile)
            fsUploadFile.write(upload.buf, upload.currentSize);
    } else if (upload.status == UPLOAD_FILE_END) {
        if (fsUploadFile)
            fsUploadFile.close();
    }
}

void handleFileDelete() {
    if (HTTP.args() == 0) return HTTP.send(500, "text/plain", "BAD ARGS");
    String path = HTTP.arg(0);
    if (path == "/")
        return HTTP.send(500, "text/plain", "BAD PATH");
    if (!FileFS.exists(path))
        return HTTP.send(404, "text/plain", "FileNotFound");
    FileFS.remove(path);
    HTTP.send(200, "text/plain", "");
    path = String();
}

void handleFileCreate() {
    if (HTTP.args() == 0)
        return HTTP.send(500, "text/plain", "BAD ARGS");
    String path = HTTP.arg(0);
    if (path == "/")
        return HTTP.send(500, "text/plain", "BAD PATH");
    if (FileFS.exists(path))
        return HTTP.send(500, "text/plain", "FILE EXISTS");
    File file = FileFS.open(path, "w");
    if (file)
        file.close();
    else
        return HTTP.send(500, "text/plain", "CREATE FAILED");
    HTTP.send(200, "text/plain", "");
    path = String();
}

void handleFileList() {
    if (!HTTP.hasArg("dir")) {
        HTTP.send(500, "text/plain", "BAD ARGS");
        return;
    }
    String path = HTTP.arg("dir");
    Dir dir = FileFS.openDir(path);
    path = String();
    String output = "[";
    while (dir.next()) {
        File entry = dir.openFile("r");
        if (output != "[") output += ',';
        bool isDir = false;
        output += "{\"type\":\"";
        output += (isDir) ? "dir" : "file";
        output += "\",\"name\":\"";
        output += String(entry.name()).substring(1);
        output += "\"}";
        entry.close();
    }
    output += "]";
    HTTP.send(200, "text/json", output);
}
#endif
#endif

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

            if (headerStr == "/config") {
                sendFileToWs5("/widgets.json", num);
                sendFileToWs5("/config.json", num);
            }

            if (headerStr == "/gifnoc") {
                writeFileUint8tByFrames("config.json", payload, length, headerLenth, 256);
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
void sendFileToWs5(const char* filename, uint8_t num) {
    sendMark(filename, "/st", num);
    size_t ws_buffer = 1024;
    String path = filepath(filename);
    auto file = FileFS.open(path, "r");
    if (!file) {
        SerialPrint(F("E"), F("FS"), F("reed file error"));
        return;
    }
    size_t fileSize = file.size();
    SerialPrint(F("i"), F("FS"), "Send file '" + String(filename) + "', file size: " + String(fileSize));
    uint8_t payload[ws_buffer + 1];
    int countRead = file.read(payload, sizeof(payload) - 1);
    while (countRead > 0) {
        payload[countRead] = 0;
        standWebSocket.sendBIN(num, payload, countRead);
        countRead = file.read(payload, sizeof(payload) - 1);
    }
    sendMark(filename, "/end", num);
}

void sendMark(const char* filename, const char* mark, uint8_t num) {
    char outChar[strlen(filename) + strlen(mark) + 1];
    strcpy(outChar, mark);
    strcat(outChar, filename);
    size_t size = strlen(outChar);
    uint8_t outUint[size];
    for (size_t i = 0; i < size; i++) {
        outUint[i] = uint8_t(outChar[i]);
    }
    standWebSocket.sendBIN(num, outUint, sizeof(outUint));
}

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
