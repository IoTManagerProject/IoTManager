#pragma once

#include "Consts.h"

#define FILE_READ "r"
#define FILE_WRITE "w"
#define FILE_APPEND "a"

#if USE_LITTLEFS
#include <LittleFS.h>
extern FS LittleFS;
using littlefs_impl::LittleFSConfig;
extern FS *filesystem;
#define FileFS LittleFS
#define FS_NAME "LittleFS"
#else
extern FS *filesystem;
#define FileFS SPIFFS
#define FS_NAME "SPIFFS"
#endif

/*
* Информация о ФС
    size_t totalBytes; // всего
    size_t usedBytes; // использовано
    size_t maxOpenFiles; // лимит на открые файлы
    size_t maxPathLength; // лимит на полное пути + имя файла

    FSInfo buf;
    getInfo(buf);
    size_t freeBytes = buf.totalBytes - buf.usedBytes;
    float freePer = buf.usedBytes / buf.totalBytes * 100;
*/ 
bool getInfo(FSInfo& info) {
    return FileFS.info(info);
}