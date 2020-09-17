#pragma once

#include <Arduino.h>

extern void upgradeInit();
extern void getLastVersion();
extern void upgrade_firmware(int type);
extern bool upgradeFS();
extern bool upgradeBuild();
extern void restartEsp();