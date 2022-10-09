#include "StandWebServer.h"
#ifdef STANDARD_WEB_SERVER

File fsUploadFile;

void standWebServerInit() {
    //  Кэшировать файлы для быстрой работы
    HTTP.serveStatic("/bundle.js", FileFS, "/", "max-age=31536000");      // кеширование на 1 год
    HTTP.serveStatic("/bundle.css", FileFS, "/", "max-age=31536000");     // кеширование на 1 год
    HTTP.serveStatic("/bundle.js.gz", FileFS, "/", "max-age=31536000");   // кеширование на 1 год
    HTTP.serveStatic("/bundle.css.gz", FileFS, "/", "max-age=31536000");  // кеширование на 1 год
    HTTP.serveStatic("/favicon.png", FileFS, "/", "max-age=31536000");    // кеширование на 1 год

    // HTTP.on("/devicelist.json", HTTP_GET, []() {
    //     HTTP.send(200, "application/json", devListHeapJson);
    // });
    HTTP.on("/settings.h.json", HTTP_GET, []() {
        HTTP.send(200, "application/json", settingsFlashJson);
    });
    // HTTP.on("/settings.f.json", HTTP_GET, []() {
    //     HTTP.send(200, "application/json", readFile(F("settings.json"), 20000));
    // });
    // HTTP.on("/params.json", HTTP_GET, []() {
    //     String json = getParamsJson();
    //     HTTP.send(200, "application/json", json);
    // });
    // HTTP.on("/errors.json", HTTP_GET, []() {
    //     HTTP.send(200, "application/json", errorsHeapJson);
    // });
    // HTTP.on("/config.json", HTTP_GET, []() {
    //     HTTP.send(200, "application/json", readFile(F("config.json"), 20000));
    // });
    // HTTP.on("/layout.json", HTTP_GET, []() {
    //     HTTP.send(200, "application/json", readFile(F("layout.json"), 20000));
    // });
    // HTTP.on("/restart", HTTP_GET, []() {
    //     // ESP.restart();
    //     HTTP.send(200, "text/plain", "ok");
    // });

    //  Добавляем функцию Update для перезаписи прошивки по WiFi при 1М(256K FileFS) и выше
    //  httpUpdater.setup(&HTTP);

    //  Запускаем HTTP сервер
    HTTP.begin();

    //#ifdef REST_FILE_OPERATIONS
    // SPIFFS.begin();
    // {
    //     Dir dir = SPIFFS.openDir("/");
    //     while (dir.next()) {
    //         String fileName = dir.fileName();
    //         size_t fileSize = dir.fileSize();
    //     }
    // }
    // HTTP страницы для работы с FFS

    // list directory
    HTTP.on("/list", HTTP_GET, handleFileList);

    //загрузка редактора editor
    HTTP.on("/edit", HTTP_GET, []() {
        if (!HTTP.args()) {
            if (!handleFileRead("/edit.htm")) HTTP.send(404, "text/plain", "FileNotFound");
        }

        if (HTTP.hasArg("list")) {
            handleFileList();
        }

        if (HTTP.hasArg("edit")) {
            if (!handleFileRead(HTTP.arg("edit"))) HTTP.send(404, "text/plain", "FileNotFound");
        }

        if (HTTP.hasArg("download")) {
            if (!handleFileRead(HTTP.arg("download"))) HTTP.send(404, "text/plain", "FileNotFound");
        }
    });

    //Создание файла
    HTTP.on("/edit", HTTP_PUT, handleFileCreate);

    //Удаление файла
    HTTP.on("/edit", HTTP_DELETE, handleFileDelete);

    //Изменение файла
    HTTP.on(
        "/edit", HTTP_POST, []() {
            HTTP.send(200, "text/plain", "");
        },
        handleFileUpload);

    // called when the url is not defined here
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
        if (contentType == "application/octet-stream")
            HTTP.sendHeader("Content-Disposition", "attachment;filename=" + (String)file.name());
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
    else if (filename.endsWith(".txt"))
        return "text/plain";
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
    File dir = FileFS.open("/", "r");
    String output = "[";
    File entry;
    while (entry = dir.openNextFile()) {
        if (output != "[") output += ',';
        bool isDir = entry.isDirectory();
        output += "{\"type\":\"";
        output += (isDir) ? "dir" : "file";
        output += "\",\"name\":\"";
        output += String(entry.name());
        output += "\"}";
        entry.close();
    }
    output += "]";
    Serial.println(output);
    HTTP.send(200, "text/json", output);
}

void printDirectory(File dir, String& out) {
    while (true) {
        File entry = dir.openNextFile();
        if (!entry) {
            break;
        }
        if (entry.isDirectory()) {
            out += entry.name();
            out += "/";
            printDirectory(entry, out);
        } else {
            out += entry.name();
            out += "\r\n";
        }
    }
}

#endif
