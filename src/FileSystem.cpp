#include "FileSystem.h"
#include "Global.h"

void getFSInfo() {
    // Информация о ФС
    size_t totalBytes; // всего
    size_t usedBytes; // использовано
    size_t maxOpenFiles; // лимит на открые файлы
    size_t maxPathLength; // лимит на полное пути + имя файла

    FSInfo buf;
    getInfo(buf);
    size_t freeBytes = buf.totalBytes - buf.usedBytes;
    float freePer = buf.usedBytes / buf.totalBytes * 100;

    jsonWriteInt(configSetupJson, F("freeBytes"), freeBytes);
    jsonWriteFloat(configSetupJson, F("freePer"), freePer);
}

bool getInfo(FSInfo& info) {
    return FileFS.info(info);
}