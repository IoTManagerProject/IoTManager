#include "Global.h"

/*********************************************************************************************************************
*****************************************глобальные объекты классов***************************************************
**********************************************************************************************************************/

TickerScheduler ts(PTASK + 1);
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
IoTGpio IoTgpio(0);

String settingsFlashJson = "{}";  //переменная в которой хранятся все настройки, находится в оперативной памяти и синхронизированна с flash памятью
String errorsHeapJson = "{}";     //переменная в которой хранятся все ошибки, находится в оперативной памяти только

String paramsFlashJson = "{}";  //переменная в которой хранятся все параметры, находится в оперативной памяти и синхронизированна с flash памятью
String paramsHeapJson = "{}";   //переменная в которой хранятся все параметры, находится в оперативной памяти только

// buf
String orderBuf = "";
String eventBuf = "";

// wifi
String ssidListHeapJson = "{}";

String devListHeapJson;
String thisDeviceJson;

// Mqtt
String mqttServer = "";
int mqttPort = 0;
String mqttPrefix = "";
String mqttUser = "";
String mqttPass = "";

unsigned long mqttUptime = 0;
unsigned long flashWriteNumber = 0;

String chipId = "";
String prex = "";
String all_widgets = "";
String scenario = "";
String mqttRootDevice = "";

// DynamicJsonDocument settingsFlashJsonDoc(JSON_BUFFER_SIZE);
// DynamicJsonDocument paramsFlashJsonDoc(JSON_BUFFER_SIZE);
// DynamicJsonDocument paramsHeapJsonDoc(JSON_BUFFER_SIZE);