#include "main.h"

File seekFile(const String& filename, size_t position = 0);
const String writeFile(const String& filename, const String& str);
const String readFile(const String& filename, size_t max_size);
const String filepath(const String& filename);
bool fileSystemInit();
String prettyBytes(size_t size);
bool cutFile(const String& src, const String& dst);