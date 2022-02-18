#include "UpgradeFirm.h"

updateFirm update;

void upgrade_firmware(int type) {
    putUserDataToRam();

    // only build
    if (type == 1) {
        if (upgradeBuild()) {
            saveUserDataToFlash();
            restartEsp();
        }
    }

    // only littlefs
    else if (type == 2) {
        if (upgradeFS()) {
            saveUserDataToFlash();
            restartEsp();
        }
    }

    // littlefs and build
    else if (type == 3) {
        if (upgradeFS()) {
            saveUserDataToFlash();
            if (upgradeBuild()) {
                restartEsp();
            }
        }
    }
}

bool upgradeFS() {
    bool ret = false;
    WiFiClient wifiClient;
    SerialPrint("!!!", F("Update"), F("Start upgrade FS..."));
    if (getBinPath("") == "error") {
        SerialPrint("E", F("Update"), F("FS Path error"));
        return ret;
    }
#ifdef ESP8266
    ESPhttpUpdate.rebootOnUpdate(false);
    t_httpUpdate_return retFS = ESPhttpUpdate.updateFS(wifiClient, getBinPath("littlefs.bin"));
#endif
#ifdef ESP32
    httpUpdate.rebootOnUpdate(false);
    HTTPUpdateResult retFS = httpUpdate.updateSpiffs(wifiClient, getBinPath("spiffs.bin"));
#endif

    //если FS обновилась успешно
    if (retFS == HTTP_UPDATE_OK) {
        SerialPrint("!!!", F("Update"), F("FS upgrade done!"));
        ret = true;
    }

    return ret;
}

bool upgradeBuild() {
    bool ret = false;
    WiFiClient wifiClient;
    SerialPrint("!!!", F("Update"), F("Start upgrade BUILD..."));
    if (getBinPath("") == "error") {
        SerialPrint("E", F("Update"), F("Build Path error"));
        return ret;
    }
#ifdef esp8266_4mb
    ESPhttpUpdate.rebootOnUpdate(false);
    t_httpUpdate_return retBuild = ESPhttpUpdate.update(wifiClient, getBinPath("firmware.bin"));
#endif
#ifdef esp32_4mb
    httpUpdate.rebootOnUpdate(false);
    HTTPUpdateResult retBuild = httpUpdate.update(wifiClient, getBinPath("firmware.bin"));
#endif

    //если BUILD обновился успешно
    if (retBuild == HTTP_UPDATE_OK) {
        SerialPrint("!!!", F("Update"), F("BUILD upgrade done!"));
        ret = true;
    }

    return ret;
}

void restartEsp() {
    Serial.println(F("Restart ESP...."));
    delay(1000);
    ESP.restart();
}

const String getBinPath(String file) {
    String path = "error";
    int targetVersion = 0;
    String serverip;
    if (jsonRead(errorsHeapJson, F("chver"), targetVersion)) {
        if (targetVersion >= 400) {
            if (jsonRead(settingsFlashJson, F("serverip"), serverip)) {
                if (serverip != "") {
                    path = jsonReadStr(settingsFlashJson, F("serverip")) + "/iotm/" + String(FIRMWARE_NAME) + "/" + String(targetVersion) + "/" + file;
                }
            }
        }
    }
    SerialPrint("i", F("Update"), "path: " + path);
    return path;
}

void putUserDataToRam() {
    update.configJson = readFile("config.json", 4096);
    update.settingsFlashJson = readFile("settings.json", 4096);
}

void saveUserDataToFlash() {
    writeFile("/config.json", update.configJson);
    writeFile("/settings.json", update.settingsFlashJson);
}