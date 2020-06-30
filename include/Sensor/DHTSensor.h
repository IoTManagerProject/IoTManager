#pragma once

#include <DHTesp.h>

extern DHTesp dht;

extern String dhtT_value_name;
extern String dhtH_value_name;

extern const String comfortStr(ComfortState value);
extern const String perceptionStr(byte value);