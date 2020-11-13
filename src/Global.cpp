#include "Global.h"

#ifdef WS_enable
AsyncWebSocket ws;
//AsyncEventSource events;
#endif

TickerScheduler ts(TEST + 1);
WiFiClient espClient;
PubSubClient mqtt(espClient);
StringCommand sCmd;
AsyncWebServer server(80);
OneWire *oneWire;
DallasTemperature sensors;

/*
* Global vars
*/

boolean just_load = true;
boolean telegramInitBeen = false;

// Json
String configSetupJson = "{}";
String configLiveJson = "{}";
String configOptionJson = "{}";

// Mqtt
String chipId = "";
String prex = "";
String all_widgets = "";
String scenario = "";

//orders and events
String orderBuf = "";
String eventBuf = "";
String itemsFile = "";
String itemsLine = "";

//key lists and numbers
//=========================================
String impuls_KeyList = "";
int impuls_EnterCounter = -1;
//=========================================
String buttonOut_KeyList = "";
int buttonOut_EnterCounter = -1;
//=========================================

// Sensors
String sensorReadingMap10sec;
String sensorReadingMap30sec;

// Logging
String loggingKeyList;
int enter_to_logging_counter;

// Upgrade
String serverIP;

// Scenario
int scenario_line_status[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
int lastVersion;

boolean busScanFlag = false;
boolean fsCheckFlag = false;
boolean delElementFlag = false;

