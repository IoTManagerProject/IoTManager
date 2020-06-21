#pragma once

#ifdef ESP8266

#include <ESP8266HTTPClient.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266httpUpdate.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <SPIFFSEditor.h>
#include <Servo.h>
#include <WiFiUdp.h>
#ifdef MDNS_ENABLED
#include <ESP8266mDNS.h>
#endif

extern WiFiUDP Udp;

#endif