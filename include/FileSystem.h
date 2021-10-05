#pragma once

#include "Consts.h"

#define FILE_READ "r"
#define FILE_WRITE "w"
#define FILE_APPEND "a"

#include <FS.h>

#ifdef ESP32
#include <SPIFFS.h>
extern FS* filesystem;
#define FileFS SPIFFS
#define FS_NAME "SPIFFS"
#endif

#ifdef ESP8266
#if USE_LITTLEFS
#include "LittleFS.h"
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
#endif

extern void getFSInfo();
#ifdef ESP8266
extern bool getInfo(FSInfo& info);
#endif