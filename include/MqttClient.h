#pragma once

#include <Arduino.h>

#include "Config/MqttConfig.h"

namespace MqttClient {

void init();

bool isConnected();
bool connect();
void reconnect();
void loop();
void subscribe();

boolean publish(const String& topic, const String& data);
boolean publishData(const String& topic, const String& data);
boolean publishChart(const String& topic, const String& data);
boolean publishControl(String id, String topic, String state);
boolean publishStatus(const String& topic, const String& data);
boolean publishOrder(const String& topic, const String& data);

void publishWidgets();
void publishState();

void handleSubscribedUpdates(char* topic, uint8_t* payload, size_t length);
const String getStateStr();

}  // namespace MqttClient
