#pragma once

#include <Arduino.h>

#include "CommonTypes.h"

const String getPresetFile(uint8_t preset, ConfigType_t type);

const String getItemName(Item_t item);

const Item_t getPresetItem(uint8_t preset);
