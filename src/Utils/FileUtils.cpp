#include "Utils/FileUtils.h"


bool fileSystemInit() {
    if (!LittleFS.begin()) {
        Serial.println("[E] LittleFS");
        return false;
    }
    return true;
}

void removeFile(const String filename) {
    if (!LittleFS.remove(filename)) {
        Serial.printf("[E] on remove %s", filename.c_str());
    }
}

File seekFile(const String filename, size_t position) {
    auto file = LittleFS.open(filename, "r");
    if (!file) {
        Serial.printf("[E] on open %s", filename.c_str());
    }
    // поставим курсор в начало файла
    file.seek(position, SeekSet);
    return file;
}

String readFileString(const String filename, const String to_find) {
    String res = "Failed";
    auto file = LittleFS.open("/" + filename, "r");
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
    auto file = LittleFS.open("/" + filename, "a");
    if (!file) {
        return "Failed";
    }
    file.println(str);
    file.close();
    return "Write sucсess";
}

String writeFile(const String filename, const String str) {
    auto file = LittleFS.open("/" + filename, "w");
    if (!file) {
        return "Failed";
    }
    file.print(str);
    file.close();
    return "Write sucсess";
}

String readFile(const String filename, size_t max_size) {
    auto file = LittleFS.open("/" + filename, "r");
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
    auto file = LittleFS.open("/" + filename, "r");
    if (!file) {
        return "Failed";
    }
    size_t size = file.size();
    file.close();
    return String(size);
}
