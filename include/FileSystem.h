#pragma once

#include "Consts.h"

#define FILE_READ "r"
#define FILE_WRITE "w"
#define FILE_APPEND "a"

#if USE_LITTLEFS
#include <LittleFS.h>
extern FS LittleFS;
using littlefs_impl::LittleFSConfig;
extern FS* filesystem;
#define FileFS LittleFS
#define FS_NAME "LittleFS"
#else
extern FS* filesystem;
#define FileFS SPIFFS
#define FS_NAME "SPIFFS"
#endif


extern void getFSInfo();
extern bool getInfo(FSInfo& info);