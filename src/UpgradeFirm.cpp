#include "UpgradeFirm.h"

updateFirm update;

void upgrade_firmware(int type, String path) {
    putUserDataToRam();

    // only build
    if (type == 1) {
        if (upgradeBuild(path)) {
            saveUserDataToFlash();
            restartEsp();
        }
    }

    // only littlefs
    else if (type == 2) {
        if (upgradeFS(path)) {
            saveUserDataToFlash();
            restartEsp();
        }
    }

    // littlefs and build
    else if (type == 3) {
        if (upgradeFS(path)) {
            saveUserDataToFlash();
            if (upgradeBuild(path)) {
                restartEsp();
            }
        }
    }
}

bool upgradeFS(String path) {
    bool ret = false;
    WiFiClient wifiClient;
    SerialPrint("!!!", F("Update"), "Start upgrade FS... " + path);
    handleUpdateStatus(true, UPDATE_FS_IN_PROGRESS);
    if (path == "") {
        SerialPrint("E", F("Update"), F("FS Path error"));
        handleUpdateStatus(true, PATH_ERROR);
        return ret;
    }
#ifdef ESP8266
    ESPhttpUpdate.rebootOnUpdate(false);
    t_httpUpdate_return retFS = ESPhttpUpdate.updateFS(wifiClient, path + "/littlefs.bin");
#endif
#ifdef ESP32
    httpUpdate.rebootOnUpdate(false);
    // обновляем little fs с помощью метода обновления spiffs!!!!
    HTTPUpdateResult retFS = httpUpdate.updateSpiffs(wifiClient, path + "/littlefs.bin");
#endif

    // если FS обновилась успешно
    if (retFS == HTTP_UPDATE_OK) {
        SerialPrint("!!!", F("Update"), F("FS upgrade done!"));
        handleUpdateStatus(true, UPDATE_FS_COMPLETED);
        ret = true;
    } else {
        handleUpdateStatus(true, UPDATE_FS_FAILED);
        if (retFS == HTTP_UPDATE_FAILED) {
            SerialPrint("E", F("Update"), "HTTP_UPDATE_FAILED");
        } else if (retFS == HTTP_UPDATE_NO_UPDATES) {
            SerialPrint("E", F("Update"), "HTTP_UPDATE_NO_UPDATES");
        }
    }
    return ret;
}

bool upgradeBuild(String path) {
    bool ret = false;
    WiFiClient wifiClient;
    SerialPrint("!!!", F("Update"), "Start upgrade BUILD... " + path);
    handleUpdateStatus(true, UPDATE_BUILD_IN_PROGRESS);
    if (path == "") {
        SerialPrint("E", F("Update"), F("Build Path error"));
        handleUpdateStatus(true, PATH_ERROR);
        return ret;
    }
#if defined(esp8266_4mb) || defined(esp8266_16mb) || defined(esp8266_1mb) || defined(esp8266_1mb_ota) || defined(esp8266_2mb) || defined(esp8266_2mb_ota)
    ESPhttpUpdate.rebootOnUpdate(false);
    t_httpUpdate_return retBuild = ESPhttpUpdate.update(wifiClient, path + "/firmware.bin");
#endif
#ifdef ESP32
    httpUpdate.rebootOnUpdate(false);
    HTTPUpdateResult retBuild = httpUpdate.update(wifiClient, path + "/firmware.bin");
#endif

    // если BUILD обновился успешно
    if (retBuild == HTTP_UPDATE_OK) {
        SerialPrint("!!!", F("Update"), F("BUILD upgrade done!"));
        handleUpdateStatus(true, UPDATE_BUILD_COMPLETED);
        ret = true;
    } else {
        handleUpdateStatus(true, UPDATE_BUILD_FAILED);
        if (retBuild == HTTP_UPDATE_FAILED) {
            SerialPrint("E", F("Update"), "HTTP_UPDATE_FAILED");
        } else if (retBuild == HTTP_UPDATE_NO_UPDATES) {
            SerialPrint("E", F("Update"), "HTTP_UPDATE_NO_UPDATES");
        }
    }
    return ret;
}

void restartEsp() {
    Serial.println(F("Restart ESP...."));
    delay(1000);
    ESP.restart();
}

// теперь путь к обнавленю прошивки мы получаем из веб интерфейса
// const String getBinPath(String file) {
//     String path = "error";
//     int targetVersion = 0;
//     String serverip;
//     if (jsonRead(errorsHeapJson, F("chver"), targetVersion)) {
//         if (targetVersion >= 400) {
//             if (jsonRead(settingsFlashJson, F("serverip"), serverip)) {
//                 if (serverip != "") {
//                     path = jsonReadStr(settingsFlashJson, F("serverip")) + "/iotm/" + String(FIRMWARE_NAME) + "/" + String(targetVersion) + "/" + file;
//                 }
//             }
//         }
//     }
//     SerialPrint("i", F("Update"), "path: " + path);
//     return path;
// }

void putUserDataToRam() {
    update.configJson = readFile("config.json", 4096);
    update.settingsFlashJson = readFile("settings.json", 4096);
    update.layoutJson = readFile("layout.json", 4096);
    update.scenarioTxt = readFile("scenario.txt", 4096);
    update.chartsData = createDataBaseSting();
}

void saveUserDataToFlash() {
    writeFile("/config.json", update.configJson);
    writeFile("/settings.json", update.settingsFlashJson);
    writeFile("/layout.json", update.layoutJson);
    writeFile("/scenario.txt", update.scenarioTxt);
    writeDataBaseSting(update.chartsData);
}

void handleUpdateStatus(bool send, int state) {
    jsonWriteInt_(errorsHeapJson, F("upd"), state);
    if (send) sendStringToWs("errors", errorsHeapJson, -1);
}