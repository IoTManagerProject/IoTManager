#pragma once
// константы
#include "Const.h"

// внешние глобальные директории
#include <Arduino.h>
#include <ArduinoJson.h>
#include <TickerScheduler.h>
#include <PubSubClient.h>
#include <list>

#ifdef ESP32
#include "WiFi.h"
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#endif

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266httpUpdate.h>
#endif

#ifdef ASYNC_WEB_SERVER
#include <ESPAsyncWebServer.h>
#endif

#ifdef STANDARD_WEB_SERVER
#ifdef ESP8266
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#endif
#ifdef ESP32
#include <WebServer.h>
#endif
#endif

#ifdef STANDARD_WEB_SOCKETS
#include <WebSocketsServer.h>
#ifdef ESP8266
#include <Hash.h>
#endif
#endif

#include <FS.h>

// внутренние глобальные директории проекта
#include "utils/FileUtils.h"
#include "utils/JsonUtils.h"
#include "utils/SerialPrint.h"
#include "utils/StringUtils.h"
#include "PeriodicTasks.h"
#include "classes/IoTGpio.h"

/*********************************************************************************************************************
*****************************************глобальные объекты классов***************************************************
**********************************************************************************************************************/
extern IoTGpio IoTgpio;
extern IoTItem* rtcItem;
//extern IoTItem* camItem;
extern IoTItem* tlgrmItem;

extern TickerScheduler ts;
extern WiFiClient espClient;
extern PubSubClient mqtt;

#ifdef ASYNC_WEB_SERVER
extern AsyncWebServer server;
#endif

#ifdef STANDARD_WEB_SERVER
#ifdef ESP8266
extern ESP8266WebServer HTTP;
extern ESP8266HTTPUpdateServer httpUpdater;
#endif
#ifdef ESP32
extern WebServer HTTP;
#endif
#endif

#ifdef STANDARD_WEB_SOCKETS
extern WebSocketsServer standWebSocket;
#endif

/*********************************************************************************************************************
***********************************************глобальные переменные**************************************************
**********************************************************************************************************************/
extern String settingsFlashJson;
extern String valuesFlashJson;
extern String errorsHeapJson;
extern bool needSaveValues;

// buf
extern String orderBuf;
extern String eventBuf;
extern String mysensorBuf;

// wifi
extern String ssidListHeapJson;

extern String devListHeapJson;
extern String thisDeviceJson;

// Mqtt
extern String mqttServer;
extern int mqttPort;
extern String mqttPrefix;
extern String mqttUser;
extern String mqttPass;

extern unsigned long mqttUptime;
extern unsigned long flashWriteNumber;

extern unsigned long wifiUptime;

extern bool udpReceivingData;

extern String mqttRootDevice;
extern String chipId;
extern String prex;
extern String all_widgets;
extern String scenario;

// Time
struct Time_t {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day_of_week;
    uint8_t day_of_month;
    uint8_t month;
    uint16_t day_of_year;
    uint16_t year;
    unsigned long days;
    unsigned long valid;
};

extern unsigned long unixTime;
extern unsigned long unixTimeShort;
extern String prevDate;
extern bool firstTimeInit;

extern bool isTimeSynch;
extern Time_t _time_local;
extern Time_t _time_utc;
extern bool _time_isTrust;

// extern unsigned long loopPeriod;

// extern DynamicJsonDocument settingsFlashJsonDoc;
// extern DynamicJsonDocument paramsFlashJsonDoc;
// extern DynamicJsonDocument paramsHeapJsonDoc;

void scanI2C();