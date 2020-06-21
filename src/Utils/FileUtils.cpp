#include "Utils/FileUtils.h"

static const char* MODULE = "FS";

void printError(const String str) {
    Serial.printf("[E] [%s] %s\n", MODULE, str.c_str());
}

const String filepath(const String& filename) {
    return filename.startsWith("/") ? filename : "/" + filename;
}

bool fileSystemInit() {
    if (!LittleFS.begin()) {
        printError("init");
        return false;
    }
    return true;
}

void removeFile(const String filename) {
    if (!LittleFS.remove(filepath(filename))) {
        printError("remove " + filename);
    }
}

File seekFile(const String filename, size_t position) {
    auto file = LittleFS.open(filepath(filename), "r");
    if (!file) {
        printError("open " + filename);
    }
    // поставим курсор в начало файла
    file.seek(position, SeekSet);
    return file;
}

String readFileString(const String filename, const String to_find) {
    String res = "Failed";
    auto file = LittleFS.open(filepath(filename), "r");
    if (!file) {
        return "Failed";
    }
    if (file.find(to_find.c_str())) {
        res = file.readStringUntil('\n');
    }
    file.close();
    return res;
}

String addFile(const String filename, const String str) {
    auto file = LittleFS.open(filepath(filename), "a");
    if (!file) {
        return "Failed";
    }
    file.println(str);
    file.close();
    return "Sucсess";
}

String writeFile(const String filename, const String str) {
    auto file = LittleFS.open(filepath(filename), "w");
    if (!file) {
        return "Failed";
    }
    file.print(str);
    file.close();
    return "Sucсess";
}

String readFile(const String filename, size_t max_size) {
    auto file = LittleFS.open(filepath(filename), "r");
    if (!file) {
        return "Failed";
    }
    size_t size = file.size();
    if (size > max_size) {
        file.close();
        return "Large";
    }
    String temp = file.readString();
    file.close();
    return temp;
}

String getFileSize(const String filename) {
    auto file = LittleFS.open(filepath(filename), "r");
    if (!file) {
        return "Failed";
    }
    size_t size = file.size();
    file.close();
    return String(size);
}
