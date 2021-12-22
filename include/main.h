#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <StreamUtils.h>

#include "LittleFS.h"

extern FS LittleFS;
using littlefs_impl::LittleFSConfig;
extern FS* filesystem;
#define FileFS LittleFS
#define FS_NAME "LittleFS"

void setupESP();
