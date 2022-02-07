#include "Global.h"

/*********************************************************************************************************************
*****************************************глобальные объекты классов***************************************************
**********************************************************************************************************************/

TickerScheduler ts(MYTEST + 1);
WiFiClient espClient;
PubSubClient mqtt(espClient);
StringCommand sCmd;
#ifdef ASYNC_WEB_SERVER
AsyncWebServer server(80);
#endif

#ifdef STANDARD_WEB_SERVER
#ifdef ESP8266
ESP8266HTTPUpdateServer httpUpdater;
ESP8266WebServer HTTP(80);
#endif
#ifdef ESP32
WebServer HTTP(80);
#endif
#endif

#ifdef STANDARD_WEB_SOCKETS
WebSocketsServer standWebSocket = WebSocketsServer(81);
#endif

/*********************************************************************************************************************
***********************************************глобальные переменные**************************************************
**********************************************************************************************************************/

String settingsFlashJson = "{}";  //переменная в которой хранятся все настройки, находится в оперативной памяти и синхронизированна с flash памятью
String paramsFlashJson = "{}";    //переменная в которой хранятся все параметры, находится в оперативной памяти и синхронизированна с flash памятью
String paramsHeapJson = "{}";     //переменная в которой хранятся все параметры, находится в оперативной памяти только

// buf
String orderBuf = "";
String eventBuf = "";

// wifi
String ssidListJson = "{}";

String devListJson = "{}";

// Mqtt
String mqttServer = "";
int mqttPort = 0;
String mqttPrefix = "";
String mqttUser = "";
String mqttPass = "";

String chipId = "";
String prex = "";
String all_widgets = "";
String scenario = "";
String mqttRootDevice = "";

int mqttConnectAttempts = 0;
bool changeBroker = false;
int currentBroker = 1;

// DynamicJsonDocument settingsFlashJsonDoc(JSON_BUFFER_SIZE);
// DynamicJsonDocument paramsFlashJsonDoc(JSON_BUFFER_SIZE);
// DynamicJsonDocument paramsHeapJsonDoc(JSON_BUFFER_SIZE);