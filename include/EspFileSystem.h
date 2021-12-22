#pragma once

#include "Global.h"
#include "LittleFS.h"

extern FS LittleFS;
using littlefs_impl::LittleFSConfig;
extern FS* filesystem;
#define FileFS LittleFS
#define FS_NAME "LittleFS"

extern bool fileSystemInit();
extern void globalVarsSync();
