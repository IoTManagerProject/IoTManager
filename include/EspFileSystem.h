#pragma once
#include "Global.h"
#ifdef ESP32
#if USE_LITTLEFS
#include <LITTLEFS.h>
#define FileFS LittleFS
#define FS_NAME "LittleFS_32"
#else
#include <SPIFFS.h>
extern FS* filesystem;
#define FileFS SPIFFS
#define FS_NAME "SPIFFS_32"
#endif
#endif

#ifdef ESP8266
#if USE_LITTLEFS
#include "LittleFS.h"
extern FS LittleFS;
using littlefs_impl::LittleFSConfig;
extern FS* filesystem;
#define FileFS LittleFS
#define FS_NAME "LittleFS_8266"
#else
extern FS* filesystem;
#define FileFS SPIFFS
#define FS_NAME "SPIFFS_8266"
#endif
#endif

extern bool fileSystemInit();
extern void globalVarsSync();

extern String getParamsJson();

extern void syncSettingsFlashJson();

extern const String getChipId();
extern void setChipId();
extern const String getUniqueId(const char* name);
extern uint32_t ESP_getChipId(void);
extern uint32_t ESP_getFlashChipId(void);
extern const String getMacAddress();
