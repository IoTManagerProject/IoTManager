#pragma once

#ifdef ESP8266
#include <ESP8266HTTPClient.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266httpUpdate.h>
#include <Servo.h>
#include <WiFiUdp.h>

#ifdef MDNS_enable
#include <ESP8266mDNS.h>
#endif

extern WiFiUDP Udp;

#endif