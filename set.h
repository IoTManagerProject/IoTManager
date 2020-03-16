String firmware_version = "2.3+";
String new_version;

//#define OTA_enable
//#define MDNS_enable
//#define WS_enable

#define TIME_COMPILING String(__TIME__)
#define DATE_COMPILING String(__DATE__)

#define wifi_mqtt_reconnecting 20000
//-----------------------------------------------------------------
#define analog_update_int 5000
//-----------------------------------------------------------------
#define ph_shooting_interval 500 //интервал выстрела датчика
#define ph_times_to_send 10      //после скольки выстрелов делать отправку данных
//-----------------------------------------------------------------
#define temp_update_int 5000
//-----------------------------------------------------------------
#define tank_level_shooting_interval 500 //интервал выстрела датчика
#define tank_level_times_to_send 20 //после скольки выстрелов делать отправку данных
//-----------------------------------------------------------------
#define dhtT_update_int 10000
#define dhtH_update_int 10000
#define dht_calculation_update_int 10000
#define statistics_update 1000 * 60 * 60 * 4
//-----------------------------------------------------------------


//==библиотеки и объекты для ESP8266==//
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESPAsyncTCP.h>
#include <ESP8266mDNS.h>

#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPUpdateServer.h>
ESP8266HTTPUpdateServer httpUpdater;
#endif

//==библиотеки и объекты для ESP32==//
#ifdef ESP32
#include <WiFi.h>
#include <SPIFFS.h>
#ifdef MDNS_enable
#include <ESPmDNS.h>
#endif
#include <AsyncTCP.h>
#include <analogWrite.h>

#include <HTTPUpdate.h>
#include <HTTPClient.h>
//HTTPClient http;


#endif

//==общие библиотеки и объекты==//
#include <Arduino.h>
#include "time.h"
#ifdef OTA_enable
#include <ArduinoOTA.h>
#endif
#include <FS.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
AsyncWebServer server(80);
#ifdef WS_enable
AsyncWebSocket ws("/ws");
#endif
AsyncEventSource events("/events");
#include "time.h"
#include <TickerScheduler.h>
TickerScheduler ts(30);
enum {ROUTER_SEARCHING, WIFI_MQTT_CONNECTION_CHECK, LEVEL, ANALOG_, DALLAS, DHTT, DHTH, DHTC, DHTP, DHTD, STEPPER1, STEPPER2,  ANALOG_LOG, LEVEL_LOG, DALLAS_LOG, PH_LOG, CMD, TIMER_COUNTDOWN, TIMERS, TIME, STATISTICS};

//ssl//#include "dependencies/WiFiClientSecure/WiFiClientSecure.h" //using older WiFiClientSecure
#include <PubSubClient.h>
WiFiClient espClient;
//ssl//WiFiClientSecure espClient;
PubSubClient client(espClient);
//---------------------------------------------------------------
#include <StringCommand.h>
StringCommand sCmd;
//---------------------------------------------------------------
#include <Bounce2.h>
#define NUM_BUTTONS 6
boolean but[NUM_BUTTONS];
Bounce * buttons = new Bounce[NUM_BUTTONS];
//----------------------------------------------------------------
#include "GyverFilters.h" //настраивается в GyverHacks.h - MEDIAN_FILTER_SIZE
GMedian medianFilter;
//----------------------------------------------------------------
#include <OneWire.h>
#include <DallasTemperature.h>
OneWire *oneWire;
DallasTemperature sensors;
//----------------------------------------------------------------
#include "DHTesp.h"
DHTesp dht;
//----------------------------------------------------------------
#include "Adafruit_Si7021.h" //https://github.com/adafruit/Adafruit_Si7021
Adafruit_Si7021 sensor_Si7021 = Adafruit_Si7021();
//-----------------------------------------------------------------



const char* hostName = "IoT Manager";

String configSetup = "{}";
String configJson = "{}";
String optionJson = "{}";

String json = "{}";

String chipID = "";
String prefix   = "/IoTmanager";
String prex;
String ids;
//boolean busy;
String all_vigets = "";
String scenario;

String order_loop;

boolean flagLoggingAnalog = false;
boolean flagLoggingLevel = false;
boolean flagLoggingDallas = false;
boolean flagLoggingPh = false;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

const String ssdpS = "SSDP";
String current_time;

int scenario_line_status [] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

int wifi_lost_error = 0;
int mqtt_lost_error = -1;

String var;

boolean upgrade_flag = false;

boolean get_url_flag = false;
