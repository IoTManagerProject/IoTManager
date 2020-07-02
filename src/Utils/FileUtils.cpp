#include "Utils/FileUtils.h"
#include "Utils/PrintMessage.h"

static const char* MODULE = "FS";

const String filepath(const char* filename) {
    String res = String(filename);
    return res.startsWith("/") ? res : "/" + res;
}

bool fileSystemInit() {
    if (!LittleFS.begin()) {
        pm.error("init");
        return false;
    }
    return true;
}

void removeFile(const String filename) {
    String path = filepath(filename.c_str());
    if (LittleFS.exists(path)) {
        if (!LittleFS.remove(path)) {
            pm.error("remove " + path);
        }
    } else {
        pm.info("not exist" + path);
    }
}

File seekFile(const String filename, size_t position) {
    String path = filepath(filename.c_str());
    auto file = LittleFS.open(path, "r");
    if (!file) {
        pm.error("open " + path);
    }
    // поставим курсор в начало файла
    file.seek(position, SeekSet);
    return file;
}

const String readFileString(const String filename, const String to_find) {
    String path = filepath(filename.c_str());
    String res = "failed";
    auto file = LittleFS.open(path, "r");
    if (!file) {
        return "failed";
    }
    if (file.find(to_find.c_str())) {
        res = file.readStringUntil('\n');
    }
    file.close();
    return res;
}

const String addFile(const String filename, const String str) {
    String path = filepath(filename.c_str());
    auto file = LittleFS.open(path, "a");
    if (!file) {
        return "failed";
    }
    file.println(str);
    file.close();
    return "sucсess";
}

bool copyFile(const String src, const String dst, bool overwrite) {
    String srcPath = filepath(src.c_str());
    String dstPath = filepath(dst.c_str());
    pm.info("copy " + srcPath + " to " + dstPath);
    if (!LittleFS.exists(srcPath)) {
        pm.error("not exist: " + srcPath);
        return false;
    }
    if (LittleFS.exists(dstPath)) {
        if (!overwrite) {
            pm.error("already exist: " + dstPath);
            return false;
        }
        LittleFS.remove(dstPath);
    }
    auto srcFile = LittleFS.open(srcPath, "r");
    auto dstFile = LittleFS.open(dstPath, "w");

    uint8_t buf[512];
    while (srcFile.available()) {
        size_t len = srcFile.read(buf, 512);
        dstFile.write(buf, len);
    }
    srcFile.close();
    dstFile.close();
    return true;
}

const String writeFile(const String filename, const String str) {
    String path = filepath(filename.c_str());
    auto file = LittleFS.open(path, "w");
    if (!file) {
        return "failed";
    }
    file.print(str);
    file.close();
    return "sucсess";
}

bool readFile(const char* filename, String& str, size_t max_size) {
    String path = filepath(filename);
    pm.info("read " + path);
    auto file = LittleFS.open(path, "r");
    if (!file) {
        return false;
    }
    size_t size = file.size();
    if (size > max_size) {
        file.close();
        return false;
    }
    str = file.readString();
    file.close();
    return true;
}

const String getFileSize(const String filename) {
    String filepath(filename);
    auto file = LittleFS.open(filepath, "r");
    if (!file) {
        return "failed";
    }
    size_t size = file.size();
    file.close();
    return String(size);
}

const String getFSSizeInfo() {
    String res;
#ifdef ESP8266
    FSInfo info;
    if (LittleFS.info(info)) {
        res = prettyBytes(info.usedBytes) + " of " + prettyBytes(info.totalBytes);
    } else {
        res = "error";
    }
#else
    res = prettyBytes(LittleFS.usedBytes()) + " of " + prettyBytes(LittleFS.totalBytes());
#endif
    return res;
}

const String getConfigFile(uint8_t preset, ConfigType_t type) {
    char buf[64];
    sprintf(buf, "/conf/%s%03d.txt", (type == CT_CONFIG) ? "c" : "s", preset);
    return String(buf);
}