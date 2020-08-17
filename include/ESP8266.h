#pragma once

#ifdef ESP8266
#include <ESP8266WebServer.h>
#include <ESP8266httpUpdate.h>
#include "ESPAsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include <LittleFS.h>
#include <SPIFFSEditor.h>
#include <Servo.h>
#include <WiFiUdp.h>
#include <SoftwareSerial.h>
#ifdef MDNS_ENABLED
#include <ESP8266mDNS.h>
#endif

extern WiFiUDP udp;

#endif