#pragma once
//===================Libraries===================================================================================================================================================
#include <Adafruit_BME280.h>
#include <Adafruit_BMP280.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <Bounce2.h>
#include <DHTesp.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <PubSubClient.h>
#include <StringCommand.h>
#include <TickerScheduler.h>
#include <Wire.h>
#include <time.h>

#include "Clock.h"
#include "Consts.h"
#include "ESP32.h"
#include "ESP8266.h"
#include "GyverFilters.h"
#include "MqttClient.h"
#include "Upgrade.h"
#include "Utils/FileUtils.h"
#include "Utils/JsonUtils.h"
#include "Utils/SerialPrint.h"
#include "Utils/StringUtils.h"
#include "Utils/SysUtils.h"
#include "Utils/WiFiUtils.h"

#ifdef WEBSOCKET_ENABLED
extern AsyncWebSocket ws;
//extern AsyncEventSource events;
#endif


extern TickerScheduler ts;
extern WiFiClient espClient;
extern PubSubClient mqtt;
extern StringCommand sCmd;
extern AsyncWebServer server;
extern DallasTemperature sensors;
extern OneWire* oneWire;
extern boolean but[NUM_BUTTONS];
extern Bounce* buttons;

//Global vars
extern boolean just_load;

// Json
extern String configSetupJson;   //все настройки
extern String configLiveJson;    //все данные с датчиков (связан с mqtt)
extern String configOptionJson;  //для трансфера

// Mqtt
extern String chipId;
extern String prex;
extern String all_widgets;
extern String scenario;

//orders and events
extern String orderBuf;
extern String eventBuf;
extern String itemsFile;
extern String itemsLine;

//key lists and numbers
extern String impulsKeyList;
extern int impulsEnterCounter;

// Sensors
extern String sensorReadingMap10sec;
extern String sensorReadingMap30sec;
extern int8_t dallasEnterCounter;

extern String loggingKeyList;
extern int enter_to_logging_counter;
extern int scenario_line_status[40];
extern int lastVersion;

// Main
extern void setChipId();
extern void saveConfig();
extern void setConfigParam(const char* param, const String& value);

extern String getURL(const String& urls);
extern void do_fscheck();
extern void doBusScan();
extern void servo_();

extern void setLedStatus(LedStatus_t);

//Scenario
extern void eventGen(String event_name, String number);
extern String add_set(String param_name);

//Timers
extern void Timer_countdown_init();
extern void timerStart_();
extern void addTimer(String number, String time);
extern void timerStop_();
extern void delTimer(String number);
extern int readTimer(int number);

extern void upgradeInit();

// widget
extern void createWidgetByType(String widget_name, String page_name, String page_number, String file, String topic);
extern void createWidgetParam(String widget_name, String page_name, String page_number, String file, String topic, String name1, String param1, String name2, String param2, String name3, String param3);
extern void createWidget(String widget_name, String page_name, String page_number, String type, String topik);
extern void createChart(String widget_name, String page_name, String page_number, String file, String topic, String maxCount);

// PushingBox
extern void pushControl();

// UDP
//extern void udpInit();
//extern void do_udp_data_parse();
//extern void do_mqtt_send_settings_to_udp();

extern void do_update();

// Init
extern void uptime_init();

// Web
extern void web_init();

// Upgrade
extern String serverIP; 
