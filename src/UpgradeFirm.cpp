#include "Upgrade.h"

#include "Class/NotAsync.h"
#ifdef ESP8266
#include "ESP8266.h"
#else
#include <HTTPUpdate.h>
#endif

#include "Global.h"

void upgradeInit() {
    myNotAsyncActions->add(
        do_UPGRADE, [&](void*) {
            upgrade_firmware(3);
        },
        nullptr);

    myNotAsyncActions->add(
        do_GETLASTVERSION, [&](void*) {
            getLastVersion();
        },
        nullptr);

    if (isNetworkActive()) {
        getLastVersion();
        if (lastVersion > 0) {
            SerialPrint("I", "Update", "available version: " + String(lastVersion));
            if (lastVersion > FIRMWARE_VERSION) {
                jsonWriteStr(configSetupJson, "warning2", F("<div style='margin-top:10px;margin-bottom:10px;'><font color='black'><p style='border: 1px solid #DCDCDC; border-radius: 3px; background-color: #ffc7c7; padding: 10px;'>Вышла новая версия прошивки, нажмите <b>обновить прошивку</b></p></font></div>"));
            }
        }
    };
}

void getLastVersion() {
    if ((WiFi.status() == WL_CONNECTED)) {
#ifdef ESP8266
        String tmp = getURL(serverIP + F("/projects/iotmanager/esp8266/esp8266ver/esp8266ver.txt"));
#else
        String tmp = getURL(serverIP + F("/projects/iotmanager/esp32/esp32ver/esp32ver.txt"));
#endif
        if (tmp == "error") {
            lastVersion = -1;
    }
        else {
            lastVersion = tmp.toInt();
        }
}
    else {
        lastVersion = -2;
    }
    jsonWriteInt(configSetupJson, "last_version", lastVersion);
}

void upgrade_firmware(int type) {
    String scenario_ForUpdate;
    String devconfig_ForUpdate;
    String configSetup_ForUpdate;

    scenario_ForUpdate = readFile(String(DEVICE_SCENARIO_FILE), 4000);
    devconfig_ForUpdate = readFile(String(DEVICE_CONFIG_FILE), 4000);
    configSetup_ForUpdate = configSetupJson;

    if (type == 1) {  //only build
        if (upgradeBuild()) restartEsp();
    }

    else if (type == 2) {  //only spiffs
        if (upgradeFS()) {
            writeFile(String(DEVICE_SCENARIO_FILE), scenario_ForUpdate);
            writeFile(String(DEVICE_CONFIG_FILE), devconfig_ForUpdate);
            writeFile("config.json", configSetup_ForUpdate);
            restartEsp();
        }
    }

    else if (type == 3) {  //spiffs and build
        if (upgradeFS()) {
            writeFile(String(DEVICE_SCENARIO_FILE), scenario_ForUpdate);
            writeFile(String(DEVICE_CONFIG_FILE), devconfig_ForUpdate);
            writeFile("config.json", configSetup_ForUpdate);
            saveConfig();
            if (upgradeBuild()) {
                restartEsp();
            }
        }
    }
}

bool upgradeFS() {
    WiFiClient wifiClient;
    bool ret = false;
    Serial.println("Start upgrade LittleFS, please wait...");
#ifdef ESP8266
    ESPhttpUpdate.rebootOnUpdate(false);
    t_httpUpdate_return retFS = ESPhttpUpdate.updateSpiffs(wifiClient, serverIP + F("/projects/iotmanager/esp8266/littlefs/littlefs.bin"));
#else
    httpUpdate.rebootOnUpdate(false);
    HTTPUpdateResult retFS = httpUpdate.updateSpiffs(wifiClient, serverIP + F("/projects/iotmanager/esp32/littlefs/spiffs.bin"));
#endif
    if (retFS == HTTP_UPDATE_OK) {  //если FS обновилась успешно
        SerialPrint("I", "Update", "LittleFS upgrade done!");
        ret = true;
}
    return ret;
}

bool upgradeBuild() {
    WiFiClient wifiClient;
    bool ret = false;
    Serial.println("Start upgrade BUILD, please wait...");

#ifdef ESP8266
    ESPhttpUpdate.rebootOnUpdate(false);
    t_httpUpdate_return retBuild = ESPhttpUpdate.update(wifiClient, serverIP + F("/projects/iotmanager/esp8266/firmware/firmware.bin"));
#else
    httpUpdate.rebootOnUpdate(false);
    HTTPUpdateResult retBuild = httpUpdate.update(wifiClient, serverIP + F("/projects/iotmanager/esp32/firmware/firmware.bin"));
#endif

    if (retBuild == HTTP_UPDATE_OK) {  //если BUILD обновился успешно
        SerialPrint("I", "Update", "BUILD upgrade done!");
        ret = true;
}
    return ret;
}

void restartEsp() {
    Serial.println("Restart ESP....");
    delay(1000);
    ESP.restart();
}