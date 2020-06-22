#include "Utils/FileUtils.h"
#include "Utils/PrintMessage.h"

static const char* MODULE = "FS";

const String filepath(const String& filename) {
    return filename.startsWith("/") ? filename : "/" + filename;
}

bool fileSystemInit() {
    if (!LittleFS.begin()) {
        pm.error("init");
        return false;
    }
    return true;
}

void removeFile(const String filename) {
    if (!LittleFS.remove(filepath(filename))) {
        pm.error("remove " + filename);
    }
}

File seekFile(const String filename, size_t position) {
    auto file = LittleFS.open(filepath(filename), "r");
    if (!file) {
        pm.error("open " + filename);
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

bool copyFile(const String src, const String dst, bool overwrite) {
    String source = filepath(src);
    String destination = filepath(dst);
    if (!LittleFS.exists(source)) {
        pm.error("source not exist: " + source);
        return false;
    }
    if (LittleFS.exists(destination)) {
        if (!overwrite) {
            pm.error("destination already exist: " + destination);
            return false;
        }
        LittleFS.remove(destination);
    }
    auto srcFile = LittleFS.open(source, "r");
    auto dstFile = LittleFS.open(destination, "w");

    static uint8_t buf[512];
    while (srcFile.read(buf, 512)) {
        dstFile.write(buf, 512);
    }
    srcFile.close();
    dstFile.close();
    return true;
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
