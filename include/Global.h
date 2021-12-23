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
#include <ESP8266httpUpdate.h>
#endif

#ifdef ACYNC_WEB_SERVER
#include <ESPAsyncWebServer.h>
#endif

#include <FS.h>

//внутренние глобальные директории проекта
#include "Utils/FileUtils.h"
#include "Utils/JsonUtils.h"
#include "Utils/SerialPrint.h"

//глобальные объекты классов
extern TickerScheduler ts;
#ifdef ACYNC_WEB_SERVER
extern AsyncWebServer server;
#endif

//глобальные переменные
extern String settingsFlashJson;
extern String paramsFlashJson;
extern String paramsHeapJson;