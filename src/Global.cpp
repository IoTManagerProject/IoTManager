#include "Global.h"

#ifdef WS_enable
AsyncWebSocket ws;
//AsyncEventSource events;
#endif
Clock* timeNow;
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

// Sensors
int8_t dallasEnterCounter = -1;
String sensorReadingMap10sec;
String sensorReadingMap60sec;

// Logging
String logging_value_names_list;
int enter_to_logging_counter;

// Scenario
int scenario_line_status[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
int lastVersion;

boolean busScanFlag = false;
boolean fsCheckFlag = false;
boolean delElementFlag = false;

