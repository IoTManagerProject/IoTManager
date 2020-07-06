#pragma once

#include <Arduino.h>
#include <IPAddress.h>

namespace NetworkManager {

void init();
bool isNetworkActive();
void startSTAMode();
bool startAPMode();
bool scanWiFi(String ssid);
IPAddress getHostIP();
}