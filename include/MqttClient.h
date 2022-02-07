#pragma once

#include <Arduino.h>


extern void sendMqttMsg();
void mqttInit();
void selectBroker();
void getMqttData1();
void getMqttData2();
bool isSecondBrokerSet();
boolean mqttConnect();
void mqttReconnect();
void mqttLoop();
void mqttSubscribe();

boolean publish(const String& topic, const String& data);
boolean publishData(const String& topic, const String& data);
boolean publishChart(const String& topic, const String& data);
boolean publishControl(String id, String topic, String state);
boolean publishChart_test(const String& topic, const String& data);
boolean publishStatus(const String& topic, const String& data);
boolean publishEvent(const String& topic, const String& data);
boolean publishInfo(const String& topic, const String& data);
boolean publishAnyJsonKey(const String& topic, const String& key, const String& data);

void publishWidgets();
void publishState();

void mqttCallback(char* topic, uint8_t* payload, size_t length);
const String getStateStr();

String getAllJson();
