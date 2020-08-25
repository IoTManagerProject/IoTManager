#pragma once

#include <Arduino.h>

const String getAvailableUrl(const char* mcu);
void getLastVersion();
void upgradeInit();
void upgrade_firmware();