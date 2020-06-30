#pragma once

#include <Arduino.h>

namespace Broadcast {
void init();
void loop();
void handleUdp_esp32();
void addKnownDevice(String fileName, String id, String dev_name, String ip);

void send_mqtt_settings();
}  // namespace Broadcast
