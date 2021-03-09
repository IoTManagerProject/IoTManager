#include "Class/NotAsync.h"
#include "FileSystem.h"
#include "Upgrade.h"
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
    SerialPrint("I", F("Update"), F("Updater Init"));
}

void getLastVersion() {
    if ((WiFi.status() == WL_CONNECTED)) {
        String tmp;
#ifdef esp8266_4mb
        tmp = getURL(serverIP + F("/projects/iotmanager/esp8266/esp8266ver/esp8266ver.txt"));
#endif        
#ifdef esp32_4mb
        tmp = getURL(serverIP + F("/projects/iotmanager/esp32/esp32ver/esp32ver.txt"));
#endif
#ifdef esp8266_mysensors_4mb
        tmp = getURL(serverIP + F("/projects/iotmanager/esp8266ms/esp8266ver/esp8266ver.txt"));
#endif        
#ifdef esp32_mysensors_4mb
        tmp = getURL(serverIP + F("/projects/iotmanager/esp32ms/esp32ver/esp32ver.txt"));
#endif
        if (tmp == "error") {
            lastVersion = -1;
        } else {
            lastVersion = tmp.toInt();
        }
    } else {
        lastVersion = -2;
    }
    jsonWriteInt(configSetupJson, "last_version", lastVersion);
}

void upgrade_firmware(int type) {
    String scenario_ForUpdate;
    String devconfig_ForUpdate;
    String configSetup_ForUpdate;

    scenario_ForUpdate = readFile(String(DEVICE_SCENARIO_FILE), 4096);
    devconfig_ForUpdate = readFile(String(DEVICE_CONFIG_FILE), 4096);
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
#ifndef esp8266_1mb
    WiFiClient wifiClient;
    bool ret = false;
    Serial.printf("Start upgrade %s, please wait...", FS_NAME);
#ifdef esp8266_4mb
    ESPhttpUpdate.rebootOnUpdate(false);
    t_httpUpdate_return retFS = ESPhttpUpdate.updateSpiffs(wifiClient, serverIP + F("/projects/iotmanager/esp8266/littlefs/littlefs.bin"));
#endif
#ifdef esp32_4mb
    httpUpdate.rebootOnUpdate(false);
    HTTPUpdateResult retFS = httpUpdate.updateSpiffs(wifiClient, serverIP + F("/projects/iotmanager/esp32/littlefs/spiffs.bin"));
#endif
#ifdef esp8266_mysensors_4mb
    ESPhttpUpdate.rebootOnUpdate(false);
    t_httpUpdate_return retFS = ESPhttpUpdate.updateSpiffs(wifiClient, serverIP + F("/projects/iotmanager/esp8266ms/littlefs/littlefs.bin"));
#endif
#ifdef esp32_mysensors_4mb
    httpUpdate.rebootOnUpdate(false);
    HTTPUpdateResult retFS = httpUpdate.updateSpiffs(wifiClient, serverIP + F("/projects/iotmanager/esp32ms/littlefs/spiffs.bin"));
#endif
    if (retFS == HTTP_UPDATE_OK) {  //если FS обновилась успешно
        SerialPrint("I", "Update", "FS upgrade done!");
        ret = true;
    }
    return ret;
#endif
}

bool upgradeBuild() {
#ifndef esp8266_1mb
    WiFiClient wifiClient;
    bool ret = false;
    Serial.println("Start upgrade BUILD, please wait...");

#ifdef esp8266_4mb
    ESPhttpUpdate.rebootOnUpdate(false);
    t_httpUpdate_return retBuild = ESPhttpUpdate.update(wifiClient, serverIP + F("/projects/iotmanager/esp8266/firmware/firmware.bin"));
#endif
#ifdef esp32_4mb
    httpUpdate.rebootOnUpdate(false);
    HTTPUpdateResult retBuild = httpUpdate.update(wifiClient, serverIP + F("/projects/iotmanager/esp32/firmware/firmware.bin"));
#endif
#ifdef esp8266_mysensors_4mb
    ESPhttpUpdate.rebootOnUpdate(false);
    t_httpUpdate_return retBuild = ESPhttpUpdate.update(wifiClient, serverIP + F("/projects/iotmanager/esp8266ms/firmware/firmware.bin"));
#endif
#ifdef esp32_mysensors_4mb
    httpUpdate.rebootOnUpdate(false);
    HTTPUpdateResult retBuild = httpUpdate.update(wifiClient, serverIP + F("/projects/iotmanager/esp32ms/firmware/firmware.bin"));
#endif

    if (retBuild == HTTP_UPDATE_OK) {  //если BUILD обновился успешно
        SerialPrint("I", "Update", "BUILD upgrade done!");
        ret = true;
    }
    return ret;
#endif
}

void restartEsp() {
    Serial.println("Restart ESP....");
    delay(1000);
    ESP.restart();
}