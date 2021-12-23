#pragma once
//константы
#include "Const.h"

//внешние глобальные директории
#include <Arduino.h>
#include <ArduinoJson.h>
#include <TickerScheduler.h>

#ifdef ESP32
#include <WiFi.h>
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
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#endif

#include <FS.h>

//внутренние глобальные директории проекта
#include "Utils/FileUtils.h"
#include "Utils/JsonUtils.h"
#include "Utils/SerialPrint.h"

//глобальные объекты классов
extern TickerScheduler ts;
#ifdef ASYNC_WEB_SERVER
extern AsyncWebServer server;
#endif

#ifdef STANDARD_WEB_SERVER
extern ESP8266WebServer HTTP;
extern ESP8266HTTPUpdateServer httpUpdater;
#endif

//глобальные переменные
extern String settingsFlashJson;
extern String paramsFlashJson;
extern String paramsHeapJson;