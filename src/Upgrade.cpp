#include "Upgrade.h"

#include "Global.h"
#include "ESP8266.h"

static const char* MODULE = "Update";

static const char* check_update_url PROGMEM = "http://91.204.228.124:1100/update/%s/version.txt";

const String getAvailableUrl(const char* mcu) {
    char buf[128];
    sprintf_P(buf, check_update_url, mcu);
    return buf;
}

void getLastVersion() {
    if (checkUpdatesFlag) {
        String url;
#ifdef ESP8266
        url = getAvailableUrl("esp8266");
#else
        url = getAvailableUrl("esp32");
#endif
        lastVersion = getURL(url);
        jsonWriteStr(configSetupJson, "last_version", lastVersion);
        checkUpdatesFlag = false;
    }
}

void initUpdater() {
    if (isNetworkActive()) {
        getLastVersion();
        if (lastVersion.length()) {
            pm.info("available: " + lastVersion);
        }
    };
}

void upgrade_firmware() {
    String scanerioBackup, configBackup, setupBackup;

    scanerioBackup = readFile(String(DEVICE_SCENARIO_FILE), 4096);
    configBackup = readFile(String(DEVICE_CONFIG_FILE), 4096);
    setupBackup = configSetupJson;

    pm.info(String("update data"));
    WiFiClient wifiClient;
#ifdef ESP8266
    ESPhttpUpdate.rebootOnUpdate(false);
    t_httpUpdate_return ret = ESPhttpUpdate.updateSpiffs(wifiClient, "http://91.204.228.124:1100/update/esp8266/esp32-esp8266_iot-manager_modules_firmware.spiffs.bin");
#else
    httpUpdate.rebootOnUpdate(false);
    t_httpUpdate_return ret = httpUpdate.updateSpiffs(client_for_upgrade, "http://91.204.228.124:1100/update/esp32/esp32-esp8266_iot-manager_modules_firmware.spiffs.bin");
#endif
    if (ret == HTTP_UPDATE_OK) {
        writeFile(String(DEVICE_SCENARIO_FILE), scanerioBackup);
        writeFile(String(DEVICE_CONFIG_FILE), configBackup);
        writeFile("config.json", setupBackup);

        saveConfig();

        pm.info(String("done!"));
    } else {
        pm.error(String("on data"));
        return;
    }

    Serial.println("update firmware");
#ifdef ESP8266
    ret = ESPhttpUpdate.update(wifiClient, "http://91.204.228.124:1100/update/esp8266/esp32-esp8266_iot-manager_modules_firmware.ino.bin");
#else
    ret = httpUpdate.update(client_for_upgrade, "http://91.204.228.124:1100/update/esp32/esp32-esp8266_iot-manager_modules_firmware.ino.bin");
#endif
    if (ret == HTTP_UPDATE_OK) {
        pm.info(String("done! restart..."));
        ESP.restart();
    } else {
        pm.error(String("on firmware"));
    }
}

void do_update() {
    if (updateFlag) {
        updateFlag = false;
        upgrade_firmware();
    }
}