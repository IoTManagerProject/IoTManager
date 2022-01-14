#pragma once
//константы
#include "Const.h"

//внешние глобальные директории
#include <Arduino.h>
#include <ArduinoJson.h>
#include <TickerScheduler.h>
#include <PubSubClient.h>
#include <StringCommand.h>

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

//внутренние глобальные директории проекта
#include "Utils/FileUtils.h"
#include "Utils/JsonUtils.h"
#include "Utils/SerialPrint.h"
#include "Utils/StringUtils.h"

/*********************************************************************************************************************
*****************************************глобальные объекты классов***************************************************
**********************************************************************************************************************/

extern TickerScheduler ts;
extern WiFiClient espClient;
extern PubSubClient mqtt;
extern StringCommand sCmd;
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
extern String paramsFlashJson;
extern String paramsHeapJson;

// buf
extern String orderBuf;
extern String eventBuf;

// Mqtt
extern String mqttServer;
extern int mqttPort;
extern String mqttPrefix;
extern String mqttUser;
extern String mqttPass;

extern String mqttRootDevice;
extern String chipId;
extern String prex;
extern String all_widgets;
extern String scenario;

extern int mqttConnectAttempts;
extern bool changeBroker;
extern int currentBroker;

// extern DynamicJsonDocument settingsFlashJsonDoc;
// extern DynamicJsonDocument paramsFlashJsonDoc;
// extern DynamicJsonDocument paramsHeapJsonDoc;