#include "utils/FileUtils.h"

//данная функция записывает файл из буфера страницами указанного размера
void writeFileUint8tByFrames(const String& filename, uint8_t*& big_buf, size_t length, size_t headerLenth, size_t frameSize) {
    String path = filepath(filename);
    auto file = FileFS.open(path, "w");
    if (!file) {
        Serial.println(F("failed write file uint8tByFrames"));
        return;
    }
    size_t written{headerLenth};
    while (length > written) {
        size_t size = length - written;
        if (size > frameSize) size = frameSize;
        uint8_t* p = &big_buf[written];
        size_t res = file.write(p, size);
        if (size != res) {
            break;
        }
        written += res;
        yield();
    }
    file.close();
    onFlashWrite();
}

// void writeStrValueToJsonFile(const String& filename, String key, String value) {
//     String tmp = readFile(filename, 4096);
//     if (!jsonWriteStr_(tmp, key, value)) {
//         Serial.println(F("failed write json value to file"));
//     }
//     writeFile(filename, tmp);
// }

//данная функция читает из файла страницами указанного размера
// void readFileUint8tByFrames(const String& filename, size_t frameSize) {
//    String path = filepath(filename);
//    auto file = FileFS.open(path, "r");
//    if (!file) {
//        Serial.println(F("failed read file Uint8tByFrames"));
//        return;
//    }
//    size_t length = file.size();
//    size_t read{0};
//    while (length > read) {
//        size_t size = length - read;
//        if (size > frameSize) size = frameSize;
//        uint8_t p[size];
//        size_t res = file.read(p, size);
//        //
//        if (size != res) {
//            break;
//        }
//        read += res;
//        yield();
//    }
//    file.close();
//}

void writeFileUint8tByByte(const String& filename, uint8_t*& payload, size_t length, size_t headerLenth) {
    String path = filepath(filename);
    auto file = FileFS.open(path, "w");
    if (!file) {
        Serial.println(F("failed write file uint8tByByte"));
        return;
    }
    size_t every = 0;
    for (size_t i = 0; i < length; i++) {
        if (i >= headerLenth) {
            every++;
            file.print((char)payload[i]);
            if (every > 256) {
                yield();
                every = 0;
            }
        }
    }
    file.close();
    onFlashWrite();
}

File seekFile(const String& filename, size_t position) {
    String path = filepath(filename);
    auto file = FileFS.open(path, "r");
    if (!file) {
        SerialPrint(F("E"), F("FS"), F("seek file error"));
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
    onFlashWrite();
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
    SerialPrint(F("i"), F("FS"), "cut " + srcPath + " to " + dstPath);
    if (!FileFS.exists(srcPath)) {
        SerialPrint(F("E"), F("FS"), "not exist: " + srcPath);
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
    onFlashWrite();
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
    onFlashWrite();
    return "sucсess";
}

void onFlashWrite() {
    flashWriteNumber++;
    SerialPrint(F("->"), F("FS"), F("write data on flash"));
}