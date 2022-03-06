#include "StandWebServer.h"

#ifdef STANDARD_WEB_SERVER
File fsUploadFile;
#include <detail/mimetable.h>

void standWebServerInit() {
    //  Кэшировать файлы для быстрой работы
    HTTP.serveStatic("/css/", FileFS, "/css/", "max-age=31536000");  // кеширование на 1 год
    HTTP.serveStatic("/js/", FileFS, "/js/", "max-age=31536000");    // кеширование на 1 год
    HTTP.serveStatic("/png/", FileFS, "/png/", "max-age=31536000");  // кеширование на 1 год

    HTTP.on("/settings.json", HTTP_GET, []() {
        HTTP.send(200, "application/json", settingsFlashJson);
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
    String pathWithGz = path + ".gz";
    if (FileFS.exists(pathWithGz) || FileFS.exists(path)) {
        if (FileFS.exists(pathWithGz))
            path += ".gz";
        File file = FileFS.open(path, "r");
        HTTP.streamFile(file, contentType(path));
        file.close();
        return true;
    }
    return false;
}

String contentType(const String& filename) {
    String res = mime::mimeTable[mime::none].mimeType;
    if (!HTTP.hasArg("download"))
        for (size_t i = 0; i < mime::maxType; i++) {
            if (filename.endsWith(mime::mimeTable[i].endsWith)){
                res = mime::mimeTable[i].mimeType;
                break;
            }
        }
    return res;
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

    void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
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
                // Serial.printf("[%u] get Text: %s\n", num, payload);

                String payloadStr;
                payloadStr.reserve(length + 1);
                for (size_t i = 0; i < length; i++) {
                    payloadStr += (char)payload[i];
                }

                if (payloadStr.startsWith("/config")) {
                    //если прилетел url страницы /config то отправим widgets.json и config.json

                    // не нравится мне это, нужно что бы класс строил очередь и отправлял вначале первый файл потом второй
                    // очередь задавалась бы так: /widgets.json,/config.json,
                    // хорошая идея на завтра)

                    if (sendWigdetsJson) sendWigdetsJson->sendFile("/widgets.json", num);
                    if (sendConfigJson) sendConfigJson->sendFile("/config.json", num);
                }

                if (payloadStr.startsWith("/gifnoc.json")) {  //если прилетел измененный пакет с меткой /gifnoc (config наоборот) то перепишем файл, пока переписываем целеком
                    payloadStr.replace("/gifnoc.json", "");
                    writeFile(F("config.json"), payloadStr);
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
