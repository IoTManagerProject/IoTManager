#include "Global.h"

//глобальные объекты классов
TickerScheduler ts(MYTEST + 1);
#ifdef ASYNC_WEB_SERVER
AsyncWebServer server(80);
#endif

#ifdef STANDARD_WEB_SERVER
ESP8266HTTPUpdateServer httpUpdater;
ESP8266WebServer HTTP(80);
#endif

//глобальные переменные
String settingsFlashJson = "{}";  //переменная в которой хранятся все настройки, находится в оперативной памяти и синхронизированна с flash памятью
String paramsFlashJson = "{}";    //переменная в которой хранятся все параметры, находится в оперативной памяти и синхронизированна с flash памятью
String paramsHeapJson = "{}";     //переменная в которой хранятся все параметры, находится в оперативной памяти только