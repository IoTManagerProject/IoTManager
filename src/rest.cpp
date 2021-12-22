#include "rest.h"

File seekFile(const String& filename, size_t position) {
    String path = filepath(filename);
    auto file = FileFS.open(path, "r");
    if (!file) {
        Serial.println("[E] file error");
    }
    file.seek(position, SeekSet);
    return file;
}

const String writeFile(const String& filename, const String& str) {
    String path = filepath(filename);
    auto file = FileFS.open(path, "w");
    if (!file) {
        return "failed";
    }
    file.print(str);
    file.close();
    return "sucсess";
}

const String readFile(const String& filename, size_t max_size) {
    String path = filepath(filename);
    auto file = FileFS.open(path, "r");
    if (!file) {
        return "failed";
    }
    size_t size = file.size();
    if (size > max_size) {
        file.close();
        return "large";
    }
    String temp = file.readString();
    file.close();
    return temp;
}

const String filepath(const String& filename) {
    return filename.startsWith("/") ? filename : "/" + filename;
}

bool fileSystemInit() {
    if (!FileFS.begin()) {
        Serial.println("FS Init ERROR, may be FS was not flashed");
        return false;
    }
    Serial.println("FS Init completed");
    return true;
}

String prettyBytes(size_t size) {
    if (size < 1024)
        return String(size) + "b";
    else if (size < (1024 * 1024))
        return String(size / 1024.0) + "kB";
    else if (size < (1024 * 1024 * 1024))
        return String(size / 1024.0 / 1024.0) + "MB";
    else
        return String(size / 1024.0 / 1024.0 / 1024.0) + "GB";
}

bool cutFile(const String& src, const String& dst) {
    String srcPath = filepath(src);
    String dstPath = filepath(dst);
    Serial.println("cut " + srcPath + " to " + dstPath);
    if (!FileFS.exists(srcPath)) {
        Serial.println("not exist: " + srcPath);
        return false;
    }
    if (FileFS.exists(dstPath)) {
        FileFS.remove(dstPath);
    }
    auto srcFile = FileFS.open(srcPath, "r");
    auto dstFile = FileFS.open(dstPath, "w");

    uint8_t buf[512];
    while (srcFile.available()) {
        size_t len = srcFile.read(buf, 512);
        dstFile.write(buf, len);
    }
    srcFile.close();
    dstFile.close();
    FileFS.remove(srcPath);
    return true;
}