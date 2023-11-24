#pragma once
#include "Global.h"
// #include "Upgrade.h"
#ifdef ESP8266
// #include "ESP8266.h"
#else
#include <HTTPUpdate.h>
#endif

struct updateFirm {
    String settingsFlashJson;
    String configJson;
    String layoutJson;
    String scenarioTxt;
    String chartsData;
};

extern void upgradeInit();
extern void getLastVersion();
extern void upgrade_firmware(int type, String path);
extern bool upgradeFS(String path);
extern bool upgradeBuild(String path);
extern void restartEsp();

extern const String getBinPath(String file);
extern void putUserDataToRam();
extern void saveUserDataToFlash();
extern void saveUpdeteStatus(String key, int val);