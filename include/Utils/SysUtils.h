#pragma once

#include <Arduino.h>

const String getChipId();

const String getUniqueId(const String& name);

const String getMemoryStatus();

const String getHeapStats();

const String getMacAddress();