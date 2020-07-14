#pragma once

#include <DHTesp.h>

namespace DHTSensor {
extern DHTesp dht;
extern String dhtT_value_name;
extern String dhtH_value_name;

void dhtC_reading();
void dhtD_reading();
void dhtH_reading();
void dhtT_reading();
void dhtP_reading();

const String comfortStr(ComfortState value);
const String perceptionStr(byte value);
}