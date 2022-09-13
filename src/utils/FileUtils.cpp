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

const String addFileLn(const String& filename, const String& str) {
    String path = filepath(filename);
    auto file = FileFS.open(path, "a");
    if (!file) {
        return "failed";
    }
    file.println(str);
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

//функция считает количество строк в файле
size_t countLines(const String filename) {
    size_t cnt = -1;
    String path = filepath(filename);
    auto file = FileFS.open(path, "r");
    if (!file) {
        return cnt;
    }
    file.seek(0, SeekSet);
    size_t size = file.size();
    size_t psn;
    do {
        cnt++;
        file.readStringUntil('\r\n');
        psn = file.position();
    } while (psn < size);
    file.close();
    return cnt;
}

//удаляем файл
void removeFile(const String& filename) {
    String path = filepath(filename);
    if (FileFS.exists(path)) {
        if (!FileFS.remove(path)) {
            // SerialPrint("i", "Files", "remove " + path);
        }
    } else {
        SerialPrint("E", "Files", "not exist " + path);
    }
}

//очищаем директорию с файлами
void cleanDirectory(String path) {
#if defined(ESP8266)
    auto dir = FileFS.openDir(path);
    while (dir.next()) {
        String fname = dir.fileName();
        removeFile(path + "/" + fname);
        SerialPrint("i", "Files", path + "/" + fname + " => deleted");
    }
    onFlashWrite();
#endif
#if defined(ESP32)
    path = "/" + path;
    File root = FileFS.open(path);
    path = String();
    if (!root) {
        SerialPrint("E", "Files", "nothing to delete");
        return;
    }
    if (root.isDirectory()) {
        File file = root.openNextFile();
        while (file) {
            String fname = file.name();
            removeFile(fname);
            SerialPrint("i", "Files", fname + " => deleted");
            file = root.openNextFile();
        }
    }
#endif
}

void saveDataDB(String id, String data) {
    String path = "/db/" + id + ".txt";
    writeFile(path, data);
}

String readDataDB(String id) {
    String path = "/db/" + id + ".txt";
    return readFile(path, 2000);
}

void cleanLogs() {
    cleanDirectory("db");
    //очистка данных всех экземпляров графиков
    for (std::list<IoTItem*>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
        if ((*it)->getSubtype() == "Loging") {
            (*it)->cleanData();
        }
    }
}

//счетчик количества записей на флешь за сеанс
void onFlashWrite() {
    flashWriteNumber++;
    // SerialPrint(F("->"), F("FS"), F("write data on flash"));
}

// Создаем список файлов каталога, функции от Сергея Третьякова
#if defined(ESP8266)
String FileList(String path) {
    Dir dir = FileFS.openDir(path);
    path = String();
    String output = "[";
    while (dir.next()) {
        File entry = dir.openFile("r");
        if (output != "[") output += ',';
        bool isDir = false;
        output += "{\"type\":\"";
        output += (isDir) ? "dir" : "file";
        output += "\",\"name\":\"";
        output += String(entry.name()).substring(1);
        output += "\"}";
        entry.close();
    }
    output += "]";
    return output;
}
#else
String FileList(String path) {
    File root = FileFS.open(path);
    path = String();

    String output = "[";
    if (root.isDirectory()) {
        File file = root.openNextFile();
        while (file) {
            if (output != "[") {
                output += ',';
            }
            output += "{\"type\":\"";
            output += (file.isDirectory()) ? "dir" : "file";
            output += "\",\"name\":\"";
            // output += String(file.path()).substring(1);
            output += "\"}";
            file = root.openNextFile();
        }
    }
    output += "]";
    return output;
}
#endif

#if defined(ESP8266)
bool getInfo(FSInfo& info) {
    return FileFS.info(info);
}
// Информация о ФС
void getFSInfo() {
    FSInfo buf;
    if (getInfo(buf)) {
        size_t totalBytes = buf.totalBytes;  // всего
        size_t usedBytes = buf.usedBytes;    // использовано
        // size_t maxOpenFiles = buf.maxOpenFiles;  // лимит на открые файлы
        // size_t blockSize = buf.blockSize;
        // size_t pageSize = buf.pageSize;
        // size_t maxPathLength = buf.maxPathLength;  // лимит на пути и имена файлов
        size_t freeBytes = totalBytes - usedBytes;
        float freePer = ((float)freeBytes / totalBytes) * 100;
        jsonWriteStr(errorsHeapJson, F("freeBytes"), String(freePer) + "% (" + prettyBytes(freeBytes) + ")");
    } else {
        SerialPrint("E", F("FS"), F("FS info error"));
    }
}
#endif

#if defined(ESP32)
void getFSInfo() {
    size_t totalBytes = FileFS.totalBytes();  // всего
    size_t usedBytes = FileFS.usedBytes();    // использовано
    size_t freeBytes = totalBytes - usedBytes;
    float freePer = ((float)freeBytes / totalBytes) * 100;
    jsonWriteStr(errorsHeapJson, F("freeBytes"), String(freePer) + "% (" + prettyBytes(freeBytes) + ")");
}
#endif
