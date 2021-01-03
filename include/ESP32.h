#pragma once

#ifdef ESP32
// don't change order
#include "WiFi.h"
//

#include "ESPAsyncWebServer.h"
#include "SPIFFSEditor.h"
// don't change order
#include <AsyncUDP.h>
#include <ESP32Servo.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>

//
#include <WiFi.h>
#include <analogWrite.h>

#ifdef MDNS_ENABLED
#include <ESPmDNS.h>
#endif

extern AsyncUDP udp;

#endif