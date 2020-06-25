#pragma once

#include <Arduino.h>

#include "CommonTypes.h"

const String getConfigFile(uint8_t preset, ConfigType_t type);

const String getItemName(Item_t item);

