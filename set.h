//===============FIRMWARE SETTINGS=====================================
String firmware_version = "2.3.3";
boolean mb_4_of_memory = true;
//#define OTA_enable
//#define MDNS_enable
//#define WS_enable
//#define layout_in_ram
#define wifi_mqtt_reconnecting 20000
#define analog_update_int 5000
#define temp_update_int 5000

#define tank_level_shooting_interval 500 //интервал выстрела датчика
#define tank_level_times_to_send 20 //после скольки выстрелов делать отправку данных

#define dhtT_update_int 10000
#define dhtH_update_int 10000
#define dht_calculation_update_int 10000

#define statistics_update 1000 * 60 * 60 * 2
//======================================================================

#define TIME_COMPILING String(__TIME__)
#define DATE_COMPILING String(__DATE__)

//===============библиотеки и объекты для ESP8266========================
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266HTTPUpdateServer.h>
ESP8266HTTPUpdateServer httpUpdater;
#include <WiFiUdp.h>
WiFiUDP Udp;
#include <Servo.h>
Servo myServo1;
Servo myServo2;

#ifdef MDNS_enable
#include <ESP8266mDNS.h>
#endif

#endif
//===============библиотеки и объекты для ESP32===========================
#ifdef ESP32
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <analogWrite.h>
#include "AsyncUDP.h"
AsyncUDP udp;
#include <ESP32_Servo.h> 
Servo myServo1;
Servo myServo2;

#ifdef MDNS_enable
#include <ESPmDNS.h>
#endif

#endif
//===============общие библиотеки и объекты===============================
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
#include <time.h>

#include <TickerScheduler.h>
TickerScheduler ts(30);
enum {ROUTER_SEARCHING, WIFI_MQTT_CONNECTION_CHECK, LEVEL, ANALOG_, DALLAS, DHTT, DHTH, DHTC, DHTP, DHTD, STEPPER1, STEPPER2,  ANALOG_LOG, LEVEL_LOG, DALLAS_LOG, dhtT_LOG, dhtH_LOG, CMD, TIMER_COUNTDOWN, TIMERS, TIME, TIME_SYNC, STATISTICS, UDP, UDP_DB, TEST};

#include <PubSubClient.h>
WiFiClient espClient;

PubSubClient client(espClient);

#include <StringCommand.h>
StringCommand sCmd;

#include <Bounce2.h>
#define NUM_BUTTONS 6
boolean but[NUM_BUTTONS];
Bounce * buttons = new Bounce[NUM_BUTTONS];

#include "GyverFilters.h" //настраивается в GyverHacks.h - MEDIAN_FILTER_SIZE
GMedian medianFilter;

#include <OneWire.h>
#include <DallasTemperature.h>
OneWire *oneWire;
DallasTemperature sensors;

#include <DHTesp.h>
DHTesp dht;

#include <Wire.h>
//===============FIRMWARE VARS========================
boolean just_load = true;
const char* hostName = "IoT Manager";
//JSON
String configSetup = "{}"; 
String configJson = "{}";  
String optionJson = "{}";
//MQTT
String chipID = "";
String prex;
String all_widgets = "";
String scenario;
String order_loop;
//SENSORS
boolean flagLoggingAnalog = false;
boolean flagLoggingLevel = false;
boolean flagLoggingDallas = false;
boolean flagLoggingdhtT = false;
boolean flagLoggingdhtH = false;
//NTP
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;
String current_time;
//SCENARIO
int scenario_line_status [] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
//ERRORS
int wifi_lost_error = 0;
int mqtt_lost_error = 0;
String last_version;
//ASYNC ACTIONS
boolean upgrade_url = false;
boolean upgrade = false;
boolean mqtt_connection = false;
boolean udp_data_parse = false;
boolean mqtt_send_settings_to_udp = false;
//UDP
boolean udp_busy = false;
boolean chart_data_in_solid_array;
unsigned int udp_port = 4210;
#ifdef ESP8266
IPAddress udp_multicastIP (255, 255, 255, 255);
#endif
#ifdef ESP32
IPAddress udp_multicastIP (239, 255, 255, 255);
#endif
String received_ip;
String received_udp_line;
int udp_period;
