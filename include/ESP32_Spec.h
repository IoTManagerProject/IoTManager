#pragma once

#ifdef ESP32
#include <AsyncTCP.h>
#include <AsyncUDP.h>
#include <ESP32Servo.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <analogWrite.h>

#ifdef MDNS_enable
#include <ESPmDNS.h>
#endif

extern AsyncUDP udp;
#endif