#pragma once

#ifdef ESP8266
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdate.h>
#include "ESPAsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include <LittleFS.h>
#include <SPIFFSEditor.h>
#include <Servo.h>
#include <WiFiUdp.h>
#include <SoftwareSerial.h>
#include <ESP8266mDNS.h>

extern WiFiUDP udp;

#endif