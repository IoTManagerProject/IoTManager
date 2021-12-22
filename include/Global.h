#pragma once

//внешние глобальные директории
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266httpUpdate.h>
#include <FS.h>
#include <TickerScheduler.h>

//внутренние глобальные директории проекта
#include "Const.h"
#include "Utils/FileUtils.h"
#include "Utils/JsonUtils.h"
#include "Utils/SerialPrint.h"

//глобальные объекты классов
extern TickerScheduler ts;

//глобальные переменные
extern String settingsFlashJson;
extern String paramsFlashJson;
extern String paramsHeapJson;