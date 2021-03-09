#include "FileSystem.h"

#include "Global.h"
#ifdef ESP8266
bool getInfo(FSInfo& info) {
    return FileFS.info(info);
}

// Информация о ФС
void getFSInfo() {
    FSInfo buf;
    if (getInfo(buf)) {
        //SerialPrint("I", F("FS"), F("Get FS info completed"));

        size_t totalBytes = buf.totalBytes;      // всего
        size_t usedBytes = buf.usedBytes;        // использовано
        //size_t maxOpenFiles = buf.maxOpenFiles;  // лимит на открые файлы
        //size_t blockSize = buf.blockSize;
        //size_t pageSize = buf.pageSize;
        //size_t maxPathLength = buf.maxPathLength;  // лимит на пути и имена файлов

        size_t freeBytes = totalBytes - usedBytes;
        float freePer = ((float)freeBytes / totalBytes) * 100;

        jsonWriteStr(configSetupJson, F("freeBytes"), String(freePer) + "% (" + prettyBytes(freeBytes) + ")");

        //SerialPrint("I", F("FS"), "totalBytes=" + String(totalBytes));
        //SerialPrint("I", F("FS"), "usedBytes=" + String(usedBytes));
        //SerialPrint("I", F("FS"), "maxOpenFiles=" + String(maxOpenFiles));
        //SerialPrint("I", F("FS"), "blockSize=" + String(blockSize));
        //SerialPrint("I", F("FS"), "pageSize=" + String(pageSize));
        //SerialPrint("I", F("FS"), "maxPathLength=" + String(maxPathLength));
        //SerialPrint("I", F("FS"), "freeBytes=" + String(freeBytes));
        //SerialPrint("I", F("FS"), "freePer=" + String(freePer));
    } else {
        SerialPrint("E", F("FS"), F("FS info error"));
    }
}
#endif
