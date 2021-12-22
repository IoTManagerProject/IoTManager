#pragma once

#include "Global.h"
#include "LittleFS.h"

extern FS LittleFS;
using littlefs_impl::LittleFSConfig;
extern FS* filesystem;
#define FileFS LittleFS
#define FS_NAME "LittleFS"

extern bool fileSystemInit();

extern File seekFile(const String& filename, size_t position = 0);
extern const String writeFile(const String& filename, const String& str);
extern const String readFile(const String& filename, size_t max_size);
extern const String filepath(const String& filename);
extern bool cutFile(const String& src, const String& dst);