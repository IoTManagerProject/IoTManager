#include "Utils/FileUtils.h"

String readFileString(const String& filename, const String& str_to_found) {
    String res = "failed";
    auto file = SPIFFS.open("/" + filename, "r");
    if (file && file.find(str_to_found.c_str())) {
        res = file.readStringUntil('\n');
    }
    file.close();
    return res;
}

String addFile(const String& fileName, const String& str) {
    auto file = SPIFFS.open("/" + fileName, "a");
    if (!file) {
        return "Failed to open file";
    }
    file.println(str);
    file.close();
    return "Write sucсess";
}

String writeFile(const String& fileName, const String& str) {
    auto file = SPIFFS.open("/" + fileName, "w");
    if (!file) {
        return "Failed to open file";
    }
    file.print(str);
    file.close();
    return "Write sucсess";
}

String readFile(const String& fileName, size_t len) {
    File file = SPIFFS.open("/" + fileName, "r");
    if (!file) {
        return "Failed";
    }
    size_t size = file.size();
    if (size > len) {
        file.close();
        return "Large";
    }
    String temp = file.readString();
    file.close();
    return temp;
}

String sizeFile(const String& fileName) {
    auto file = SPIFFS.open("/" + fileName, "r");
    if (!file) {
        return "Failed";
    }
    size_t size = file.size();
    file.close();
    return String(size);
}
