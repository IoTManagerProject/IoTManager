#pragma once
#include "Global.h"
#include "EspFileSystem.h"

extern void writeFileUint8tByFrames(const String& filename, uint8_t*& big_buf, size_t length, size_t headerLenth, size_t frameSize);
extern void writeFileUint8tByByte(const String& filename, uint8_t*& payload, size_t length, size_t headerLenth);
extern File seekFile(const String& filename, size_t position = 0);
extern const String writeFile(const String& filename, const String& str);
const String writeEmptyFile(const String& filename);
extern const String addFileLn(const String& filename, const String& str);
extern const String addFile(const String& filename, const String& str);
extern const String readFile(const String& filename, size_t max_size);
extern const String filepath(const String& filename);
extern bool cutFile(const String& src, const String& dst);
extern size_t countJsonObj(const String filename, size_t& size);
void removeFile(const String& filename);
void removeDirectory(const String& dir);
void cleanDirectory(String path);
void cleanLogs();
String saveDataDB(String id, String data);
String readDataDB(String id);
extern void onFlashWrite();

String getFilesList8266(String& directory);
String getFilesList32(String& directory);
String getFilesList(String& directory);
String createDataBaseSting();
void writeDataBaseSting(String input);

struct IoTFSInfo {
    size_t totalBytes;
    float freePer;
};

extern IoTFSInfo getFSInfo();
#ifdef ESP8266
extern bool getInfo(FSInfo& info);
#endif