#pragma once
#include "Global.h"
//#include "Upgrade.h"
#ifdef ESP8266
//#include "ESP8266.h"
#else
#include <HTTPUpdate.h>
#endif

struct updateFirm {
    String settingsFlashJson;
    String configJson;
};

extern void upgradeInit();
extern void getLastVersion();
extern void upgrade_firmware(int type);
extern bool upgradeFS();
extern bool upgradeBuild();
extern void restartEsp();

extern const String getBinPath(String file);
extern void putUserDataToRam();
extern void saveUserDataToFlash();