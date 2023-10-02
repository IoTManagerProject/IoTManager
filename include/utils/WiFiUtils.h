#pragma once

#include "Global.h"
#include "MqttClient.h"
#include <vector>
// boolean isNetworkActive();
inline boolean isNetworkActive() {return WiFi.status() == WL_CONNECTED;};
void routerConnect();
bool startAPMode();
boolean RouterFind(std::vector<String> jArray);
uint8_t RSSIquality();
extern void wifiSignalInit();
