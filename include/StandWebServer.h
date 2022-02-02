#pragma once
#include "Global.h"

#ifdef STANDARD_WEB_SERVER
extern void standWebServerInit();
extern bool handleFileRead(String path);
extern String getContentType(String filename);
#ifdef REST_FILE_OPERATIONS
extern void handleFileUpload();
extern void handleFileDelete();
extern void handleFileCreate();
extern void handleFileList();
#endif
#endif
