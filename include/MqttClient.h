#pragma once
#include "Const.h"
#include "classes/NotAsync.h"
#include "Global.h"
#include "utils/WiFiUtils.h"
#include "EventsAndOrders.h"

void mqttInit();
void selectBroker();
void getMqttData();
void getMqttData2();
bool isSecondBrokerSet();
boolean mqttConnect();
void mqttReconnect();
void mqttLoop();
void mqttSubscribe();
bool mqttIsConnect();

boolean publish(const String& topic, const String& data);
boolean publishData(const String& topic, const String& data);
boolean publishChartMqtt(const String& topic, const String& data);
boolean publishJsonMqtt(const String& topic, const String& json);
boolean publishStatusMqtt(const String& topic, const String& data);
boolean publishEvent(const String& topic, const String& data);
void mqttSubscribeExternal(String topic, bool usePrefix = false);

bool publishChartFileToMqtt(String path, String id, int maxCount);

void publishWidgets();

void mqttCallback(char* topic, uint8_t* payload, size_t length);
void handleMqttStatus(bool send);
void handleMqttStatus(bool send, int state);

const String getStateStr(int e);

void mqttUptimeCalc();
void wifiUptimeCalc();