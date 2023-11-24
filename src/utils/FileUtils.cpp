#include "utils/FileUtils.h"

// данная функция записывает файл из буфера страницами указанного размера
void writeFileUint8tByFrames(const String& filename, uint8_t*& big_buf, size_t length, size_t headerLenth, size_t frameSize) {
    String path = filepath(filename);
    auto file = FileFS.open(path, FILE_WRITE);
    if (!file) {
        Serial.println(F("failed write file uint8tByFrames"));
        return;
    }
    size_t written{headerLenth};
    // SerialPrint("i", "test", filename + ": length=" + String(length) + ", written=" + String(written));
    if (length == written) file.print(" ");
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

void writeFileUint8tByByte(const String& filename, uint8_t*& payload, size_t length, size_t headerLenth) {
    String path = filepath(filename);
    auto file = FileFS.open(path, FILE_WRITE);
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
    auto file = FileFS.open(path, FILE_READ);
    if (!file) {
        SerialPrint(F("E"), F("FS"), F("seek file error"));
    }
    file.seek(position, SeekSet);
    return file;
}

const String writeFile(const String& filename, const String& str) {
    String path = filepath(filename);
#ifdef ESP32
    auto file = FileFS.open(path, FILE_WRITE, true);
#endif
#ifdef ESP8266
    auto file = FileFS.open(path, FILE_WRITE);
#endif
    if (!file) {
        return "failed";
    }
    file.print(str);
    file.close();
    return "success";
    onFlashWrite();
}

const String writeEmptyFile(const String& filename) {
    String path = filepath(filename);
#ifdef ESP32
    auto file = FileFS.open(path, FILE_WRITE, true);
#endif
#ifdef ESP8266
    auto file = FileFS.open(path, FILE_WRITE);
#endif
    if (!file) {
        return "failed";
    }
    file.close();
    return "success";
    onFlashWrite();
}

const String addFileLn(const String& filename, const String& str) {
    String path = filepath(filename);
    auto file = FileFS.open(path, FILE_APPEND);
    if (!file) {
        return "failed";
    }
    file.println(str);
    file.close();
    return "success";
    onFlashWrite();
}

const String addFile(const String& filename, const String& str) {
    String path = filepath(filename);
    auto file = FileFS.open(path, FILE_APPEND);
    if (!file) {
        return "failed";
    }
    file.print(str);
    file.close();
    return "success";
    onFlashWrite();
}

const String readFile(const String& filename, size_t max_size) {
    String path = filepath(filename);
    auto file = FileFS.open(path, FILE_READ);
    if (!file) {
        return "failed";
    }
    size_t size = file.size();
    if (size > max_size) {
        file.close();
        // что это за бред!
        if (path == "/config.json") return "[]";
        return "large";
    }
    String temp = file.readString();
    file.close();
    return temp;
}

const String filepath(const String& filename) { return filename.startsWith("/") ? filename : "/" + filename; }

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
    auto srcFile = FileFS.open(srcPath, FILE_READ);
    auto dstFile = FileFS.open(dstPath, FILE_WRITE);
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

// функция считает количество строк в файле
size_t countJsonObj(const String filename, size_t& size) {
    size_t cnt = -1;
    String path = filepath(filename);
    auto file = FileFS.open(path, FILE_READ);
    if (!file) {
        return cnt;
    }
    file.seek(0, SeekSet);
    size = file.size();
    size_t psn;
    do {
        cnt++;
        file.readStringUntil('}');
        psn = file.position();
    } while (psn < size);
    file.close();
    return cnt;
}

// удаляем файл
void removeFile(const String& filename) {
    String path = filepath(filename);
    if (FileFS.exists(path)) {
        if (!FileFS.remove(path)) {
            SerialPrint("i", "Files", "remove file" + path);
        }
    } else {
        SerialPrint("E", "Files", "file not exist " + path);
    }
}

void removeDirectory(const String& dir) {
    String path = filepath(dir);
    if (FileFS.exists(path)) {
        if (!FileFS.rmdir(path)) {
            SerialPrint("i", "Files", "remove dir" + path);
        }
    } else {
        SerialPrint("E", "Files", "dir not exist " + path);
    }
}

String saveDataDB(String id, String data) {
    String path = "/db/" + id + ".txt";
    return writeFile(path, data);
}

String readDataDB(String id) {
    String path = "/db/" + id + ".txt";
    return readFile(path, 2000);
}

void cleanLogs() {
    SerialPrint("i", "Files", "cleanLogs");
    cleanDirectory("/db");
    // обращение к логированию из ядра
    // очистка данных всех экземпляров графиков
    for (std::list<IoTItem*>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
        if ((*it)->getSubtype() == "Loging" || "LogingDaily") {
            (*it)->clearHistory();
        }
    }
}

// очищаем директорию с файлами
void cleanDirectory(String path) {
    String filesList = getFilesList(path);
    int i = 0;
    while (filesList.length()) {
        String buf = selectToMarker(filesList, ";");
        buf = path + buf;
        SerialPrint("i", "", buf);
        i++;
        removeFile(buf);
        SerialPrint("i", "Files", String(i) + ") " + buf + " => deleted");

        filesList = deleteBeforeDelimiter(filesList, ";");
    }
}

// счетчик количества записей на флешь за сеанс
void onFlashWrite() {
    flashWriteNumber++;
    // SerialPrint(F("->"), F("FS"), F("write data on flash"));
}

// Создаем список файлов каталога, функции от Сергея Третьякова
#if defined(ESP8266)
String getFilesList8266(String& directory) {
    String filesList = "";
    auto dir = FileFS.openDir(directory);
    while (dir.next()) {
        String fname = dir.fileName();
        if (fname != "") filesList += "/" + fname + ";";
    }
    return filesList;
}
#endif

#if defined(ESP32)
String getFilesList32(String& directory) {
    String filesList = "";
    File root = FileFS.open(directory);
    if (root.isDirectory()) {
        File file = root.openNextFile();
        while (file) {
            String fname = file.name();
            if (fname != "") filesList += "/" + fname + ";";
            file = root.openNextFile();
        }
    }
    return filesList;
}
#endif

String getFilesList(String& directory) {
#if defined(ESP8266)
    return getFilesList8266(directory);
#endif
#if defined(ESP32)
    return getFilesList32(directory);
#endif
}

#if defined(ESP8266)
bool getInfo(FSInfo& info) { return FileFS.info(info); }

// Информация о ФС
IoTFSInfo getFSInfo() {
    IoTFSInfo myFSInfo;
    FSInfo buf;
    if (getInfo(buf)) {
        size_t totalBytes = myFSInfo.totalBytes = buf.totalBytes;  // всего
        size_t usedBytes = buf.usedBytes;                          // использовано
        // size_t maxOpenFiles = buf.maxOpenFiles;  // лимит на открые файлы
        // size_t blockSize = buf.blockSize;
        // size_t pageSize = buf.pageSize;
        // size_t maxPathLength = buf.maxPathLength;  // лимит на пути и имена файлов
        size_t freeBytes = totalBytes - usedBytes;
        float freePer = myFSInfo.freePer = ((float)freeBytes / totalBytes) * 100;
        jsonWriteStr(errorsHeapJson, F("freeBytes"), String(freePer) + "% (" + prettyBytes(freeBytes) + ")");

    } else {
        SerialPrint("E", F("FS"), F("FS info error"));
    }
    return myFSInfo;
}
#endif

#if defined(ESP32)
IoTFSInfo getFSInfo() {
    IoTFSInfo myFSInfo;
    size_t totalBytes = myFSInfo.totalBytes = FileFS.totalBytes();  // всего
    size_t usedBytes = FileFS.usedBytes();                          // использовано
    size_t freeBytes = totalBytes - usedBytes;
    float freePer = myFSInfo.freePer = ((float)freeBytes / totalBytes) * 100;
    jsonWriteStr(errorsHeapJson, F("freeBytes"), String(freePer) + "% (" + prettyBytes(freeBytes) + ")");
    return myFSInfo;
}
#endif

String createDataBaseSting() {
    String out;
    for (std::list<IoTItem*>::iterator it = IoTItems.begin(); it != IoTItems.end(); ++it) {
        if ((*it)->getSubtype() == "LogingDaily") {
            String id = (*it)->getID();
            String path = "/lgd/" + id + "/" + id + ".txt";
            String fileContent = readFile(path, 10000);
            if (fileContent == "failed") {
                SerialPrint("i", "Export", "file not exist " + path);
            } else {
                out += "=>" + fileContent + "\r\n";
            }
        }
    }
    return out;
}

void writeDataBaseSting(String input) {
    while (input.length()) {
        String line = selectToMarker(input, "\r\n");
        String path = selectToMarker(line, "=>");
        String content = deleteBeforeDelimiter(line, "=>");
        writeFile(path, content);
        input = deleteBeforeDelimiter(input, "\r\n");
    }
}
