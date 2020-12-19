#include "FileSystem.h"
#include "Global.h"

void getFSInfo() {
    // Информация о ФС


    FSInfo buf;
    if (getInfo(buf)) {
        SerialPrint("I", F("FS"), F("Get FS info completed"));

        size_t totalBytes = buf.totalBytes; // всего
        size_t usedBytes = buf.usedBytes; // использовано
        size_t maxOpenFiles = buf.maxOpenFiles; // лимит на открые файлы
        size_t maxPathLength = buf.maxPathLength; // лимит на пути и имена файлов

        size_t freeBytes = totalBytes - usedBytes;
        float freePer = ((float) freeBytes / totalBytes) * 100;
        
        jsonWriteStr(configSetupJson, F("freeBytes"), String(freePer) + "% (" + String(freeBytes) + ")");

        SerialPrint("I", F("FS"), "totalBytes=" + String(totalBytes));
        SerialPrint("I", F("FS"), "usedBytes=" + String(usedBytes));
        SerialPrint("I", F("FS"), "maxOpenFiles=" + String(maxOpenFiles));
        SerialPrint("I", F("FS"), "maxPathLength=" + String(maxPathLength));
        SerialPrint("I", F("FS"), "freeBytes=" + String(freeBytes));
        SerialPrint("I", F("FS"), "freePer=" + String(freePer));
    }
    else {
        SerialPrint("E", F("FS"), F("FS info error"));
    }
}

bool getInfo(FSInfo& info) {
    return FileFS.info(info);
}