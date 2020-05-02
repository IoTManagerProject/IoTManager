//===============FIRMWARE SETTINGS=====================================
String firmware_version = "2.3.3";
boolean mb_4_of_memory = true;
//#define OTA_enable
//#define MDNS_enable
//#define WS_enable
//#define layout_in_ram
#define wifi_mqtt_reconnecting 30000

#define tank_level_times_to_send 10 //после скольки выстрелов делать отправку данных

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
enum {ROUTER_SEARCHING, WIFI_MQTT_CONNECTION_CHECK, SENSORS, STEPPER1, STEPPER2,  LOG1, LOG2, LOG3, LOG4, LOG5, TIMER_COUNTDOWN, TIME, TIME_SYNC, STATISTICS, UDP, UDP_DB, TEST };
TickerScheduler ts(TEST + 1);

//LEVEL, ANALOG1, ANALOG2, DALLAS, DHTT, DHTH, DHTC, DHTP, DHTD, BMP280T, BMP280P,

#include <PubSubClient.h>
WiFiClient espClient;
PubSubClient client_mqtt(espClient);

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

#include <Adafruit_BMP280.h>
Adafruit_BMP280 bmp; // use I2C interface
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();

#include <Adafruit_BME280.h>
Adafruit_BME280 bme; // use I2C interface
Adafruit_Sensor *bme_temp = bme.getTemperatureSensor();
Adafruit_Sensor *bme_pressure = bme.getPressureSensor();
Adafruit_Sensor *bme_humidity = bme.getHumiditySensor();
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
String analog_value_names_list;
int enter_to_analog_counter;

String level_value_name;
String dhtT_value_name;
String dhtH_value_name;
String bmp280T_value_name;
String bmp280P_value_name;
String bme280T_value_name;
int sensors_count = 11;
int sensors_reading_map [] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//LOGGING
String logging_value_names_list;
int enter_to_logging_counter;
//NTP and TIME
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
boolean i2c_scanning = false;
//UDP
boolean udp_busy = false;
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
//i2c
String i2c_list;
