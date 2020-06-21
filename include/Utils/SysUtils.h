#pragma once

#include <Arduino.h>

const String getChipId();

void printMemoryStatus(String text = "");

String getHeapStats();
