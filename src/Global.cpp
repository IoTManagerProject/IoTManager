#include "Global.h"

/*********************************************************************************************************************
*****************************************глобальные объекты классов***************************************************
**********************************************************************************************************************/

TickerScheduler ts(END + 1);
WiFiClient espClient;
PubSubClient mqtt(espClient);

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
IoTItem* rtcItem = nullptr;
//IoTItem* camItem = nullptr;
IoTItem* tlgrmItem = nullptr;
IoTBench* benchTaskItem = nullptr;
IoTBench* benchLoadItem = nullptr;
String settingsFlashJson = "{}";  // переменная в которой хранятся все настройки, находится в оперативной памяти и синхронизированна с flash памятью
String valuesFlashJson = "{}";    // переменная в которой хранятся все значения элементов, которые необходимо сохранить на flash. Находится в оперативной памяти и синхронизированна с flash памятью
String errorsHeapJson = "{}";     // переменная в которой хранятся все ошибки, находится в оперативной памяти только
bool needSaveValues = false;      // признак необходимости сбросить значения элементов на flash

// buf
String orderBuf = "";
String eventBuf = "";
String mysensorBuf = "";

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

unsigned long wifiUptime = 0;

bool udpReceivingData = false;

String chipId = "";
String prex = "";
String all_widgets = "";
String scenario = "";
String mqttRootDevice = "";

// Time
unsigned long unixTime = 0;
unsigned long unixTimeShort = 0;
String prevDate = "";
bool firstTimeInit = true;

// unsigned long loopPeriod;

bool isTimeSynch = false;
Time_t _time_local;
Time_t _time_utc;
bool _time_isTrust = false;

// DynamicJsonDocument settingsFlashJsonDoc(JSON_BUFFER_SIZE);
// DynamicJsonDocument paramsFlashJsonDoc(JSON_BUFFER_SIZE);
// DynamicJsonDocument paramsHeapJsonDoc(JSON_BUFFER_SIZE);