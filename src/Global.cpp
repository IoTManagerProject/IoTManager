#include "Global.h"

#ifdef WS_enable
AsyncWebSocket ws;
AsyncEventSource events;
#endif

TickerScheduler ts(MYTEST + 1);
WiFiClient espClient;
PubSubClient mqtt(espClient);
StringCommand sCmd;
AsyncWebServer server(80);

/*
 * Global vars
 */

boolean just_load = true;
boolean telegramInitBeen = false;
boolean savedFromWeb = false;
boolean wsSetupFlag = false;

// Json
String configSetupJson = "{}";
String configLiveJson = "{}";
String configStoreJson = "{}";
String configOptionJson = "{}";
String telegramMsgJson = "{}";

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

// web sockets
int wsAttempts = 100;
//char* wsBufChar = "";

// orders and events
String orderBuf = "";
String wsBuf = "";
String eventBuf = "";
String mysensorBuf = "";
String itemsFile = "";
String itemsLine = "";

// key lists and numbers
//=========================================
String impuls_KeyList = "";
int impuls_EnterCounter = -1;
//=========================================
String buttonOut_KeyList = "";
int buttonOut_EnterCounter = -1;
//=========================================
String input_KeyList = "";
int input_EnterCounter = -1;
//=========================================
String output_KeyList = "";
int output_EnterCounter = -1;
//=========================================
String pwmOut_KeyList = "";
int pwmOut_EnterCounter = -1;
//=========================================
String countDown_KeyList = "";
int countDown_EnterCounter = -1;
//=========================================
String logging_KeyList = "";
int logging_EnterCounter = -1;
//=========================================

String itemName;
String presetName;

// Upgrade
String serverIP;

// Scenario
int scenario_line_status[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
int lastVersion;

boolean busScanFlag = false;
boolean fsCheckFlag = false;
boolean delElementFlag = false;
