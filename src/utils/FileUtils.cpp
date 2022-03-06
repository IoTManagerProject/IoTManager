#include "Utils/FileUtils.h"

File seekFile(const String& filename, size_t position) {
    String path = filepath(filename);
    auto file = FileFS.open(path, "r");
    if (!file) {
       SerialPrint('E', "FS", F("seek file error"));
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

bool cutFile(const String& src, const String& dst) {
    String srcPath = filepath(src);
    String dstPath = filepath(dst);
    SerialPrint('I', "FS", "cut " + srcPath + " to " + dstPath);
    if (!FileFS.exists(srcPath)) {
       SerialPrint('E', "FS", "not exist: " + srcPath);
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

const String addFileLn(const String& filename, const String& str) {
    String path = filepath(filename);
    auto file = FileFS.open(path, "a");
    if (!file) {
        return "failed";
    }
    file.println(str);
    file.close();
    return "sucсess";
}