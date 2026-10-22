#pragma once
#include "Global.h"

// Регистрация обработчиков и сами обработчики общие для обоих вариантов сервера:
// в асинхронной сборке (ASYNC_WEB_SERVER) объект HTTP — обёртка AsyncWebServerCompat,
// повторяющая интерфейс синхронного сервера (см. WebServerCompat.h)
#if defined(STANDARD_WEB_SERVER) || defined(ASYNC_WEB_SERVER)
extern void standWebServerInit();
extern bool handleFileRead(String path);
//extern String getContentType(String filename);
//#ifdef REST_FILE_OPERATIONS
extern void handleFileUpload();
extern void handleFileDelete();
extern void handleFileCreate();
extern void handleLocalOTA();
extern void handleUpdateOTA();
extern void handleCors();
extern void handleLocalOTA_Handler();
extern void handleFileList();
//void printDirectory(File dir, String& out);
extern void handleStatus();
extern void handleGetEdit();
extern void replyOK ();
extern void handleNotFound ();

//#endif
#endif

