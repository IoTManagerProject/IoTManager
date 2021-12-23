#pragma once

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

#include <ESPAsyncWebServer.h>
#include <FS.h>

//внутренние глобальные директории проекта
#include "Const.h"
#include "Utils/FileUtils.h"
#include "Utils/JsonUtils.h"
#include "Utils/SerialPrint.h"

//глобальные объекты классов
extern TickerScheduler ts;
extern AsyncWebServer server;

//глобальные переменные
extern String settingsFlashJson;
extern String paramsFlashJson;
extern String paramsHeapJson;