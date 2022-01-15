#pragma once
#include "EspFileSystem.h"
#include "Global.h"

extern File seekFile(const String& filename, size_t position = 0);
extern const String writeFile(const String& filename, const String& str);
extern const String readFile(const String& filename, size_t max_size);
extern const String filepath(const String& filename);
extern bool cutFile(const String& src, const String& dst);
extern const String addFileLn(const String& filename, const String& str);