#pragma once

#include <Arduino.h>

namespace MqttClient {

void init();
boolean connect();
void reconnect();
void loop();

void subscribe(struct Params& p);

boolean publish(const String& topic, const String& data);
boolean publishData(const String& topic, const String& data);
boolean publishChart(const String& topic, const String& data);
boolean publishControl(const String& id, const String& topic, const String& state);
boolean publishStatus(const String& topic, const String& data);

void publishWidgets();
void publishState();

void handleSubscribedUpdates(char* topic, uint8_t* payload, size_t length);
const String getStateStr();

}  // namespace MqttClient
