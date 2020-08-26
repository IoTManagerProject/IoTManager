#pragma once
#include "HttpServer.h"
#include <Arduino.h>

extern String getURL(const String& urls);
extern const String getMethodName(AsyncWebServerRequest* request);
extern const String getRequestInfo(AsyncWebServerRequest* request);