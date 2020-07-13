#pragma once

#include <Arduino.h>
#include "Base/KeyValueStore.h"
#include "MqttWriter.h"
#include "Config/MqttConfig.h"

namespace MqttClient {

MqttWriter* getWriter(const char* topic);

void init();

bool isConnected();
bool connect();
void reconnect();
void loop();
void subscribe();

boolean mqtt_publish(const String& topic, const String& data);
boolean publishData(const String& topic, const String& data);
boolean publishChart(const String& topic, const String& data);
boolean publishControl(String id, String topic, String state);
boolean publishStatus(const String& name, const String& value, const ValueType_t type);
boolean publishOrder(const String& topic, const String& data);

void publishState();

void handleSubscribedUpdates(char* topic, uint8_t* payload, size_t length);
const String getStateStr();

}  // namespace MqttClient
