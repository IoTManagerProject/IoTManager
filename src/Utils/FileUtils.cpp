#include "Utils/FileUtils.h"

#include <LittleFS.h>

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
