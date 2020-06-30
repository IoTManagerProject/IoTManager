#include "Upgrade.h"

#include "Global.h"

#include "ESP8266.h"

#include "WebClient.h"

static const char* UPDATE_SERVER_URL PROGMEM = "http://91.204.228.124:1100/update/%s/%s";

#ifdef ESP8266
static const char* TAG_MCU = "esp8266";
#else
static const char* TAG_MCU = "esp32";
#endif
static const char* TAG_METADATA = "version.txt";
static const char* TAG_FIRMWARE_BIN = "firmware.bin";
static const char* TAG_FS_IMAGE = "fs.bin";

static const char* MODULE = "Update";

void upgrade_firmware();

const String buildUpdateUrl(const char* mcu, const char* file) {
    char buf[128];
    sprintf_P(buf, UPDATE_SERVER_URL, mcu, file);
    return buf;
}

void do_update() {
    if (updateFlag) {
        updateFlag = false;
        upgrade_firmware();
    }
}

void do_check_update() {
    if (checkUpdatesFlag) {
        String url;
#ifdef ESP8266
        url = buildUpdateUrl(TAG_MCU, TAG_METADATA);
#else
        url = buildUpdateUrl(TAG_MCU, TAG_METADATA);
#endif
        lastVersion = WebClient::get(url);
        jsonWriteStr(configSetupJson, "last_version", lastVersion);
        checkUpdatesFlag = false;
    }
}

void update_init() {
    if (isNetworkActive()) {
        do_check_update();
        if (!lastVersion.isEmpty()) {
            pm.info("available: " + lastVersion);
        }
    };
}

String scenarioBackup, configBackup, setupBackup;
void do_config_backup() {
    scenarioBackup = readFile(String(DEVICE_SCENARIO_FILE), 4096);
    configBackup = readFile(String(DEVICE_CONFIG_FILE), 4096);
    setupBackup = configSetupJson;
}

void do_config_restore() {
    writeFile(String(DEVICE_SCENARIO_FILE), scenarioBackup);
    writeFile(String(DEVICE_CONFIG_FILE), configBackup);
    writeFile("config.json", setupBackup);
    saveConfig();
}

const String getRetStr(HTTPUpdateResult ret) {
    switch (ret) {
        case HTTP_UPDATE_OK:
            return F("ok");
            break;
        case HTTP_UPDATE_NO_UPDATES:
            return F("no updates");
            break;
        case HTTP_UPDATE_FAILED:
            return F("update failed");
            break;
        default:
            return F("unknown");
    }
}

void upgrade_firmware() {
    pm.info(TAG_FS_IMAGE);
    WiFiClient wifiClient;
#ifdef ESP8266
    ESPhttpUpdate.rebootOnUpdate(false);
    HTTPUpdateResult ret = ESPhttpUpdate.updateSpiffs(wifiClient, buildUpdateUrl(TAG_MCU, TAG_FS_IMAGE));
#else
    httpUpdate.rebootOnUpdate(false);
    HTTPUpdateResult ret = httpUpdate.updateSpiffs(wifiClient, buildUpdateUrl(TAG_MCU, TAG_FS_IMAGE);
#endif
    if (ret == HTTP_UPDATE_OK) {
        pm.info(String("done!"));
        do_config_restore();
    } else {
        pm.error("ret: " + getRetStr(ret));
        return;
    }

    pm.info(TAG_FIRMWARE_BIN);
#ifdef ESP8266
    ret = ESPhttpUpdate.update(wifiClient, buildUpdateUrl(TAG_MCU, TAG_FIRMWARE_BIN));
#else
    ret = httpUpdate.update(wifiClient, buildUpdateUrl(TAG_MCU, TAG_FIRMWARE_BIN));
#endif
    if (ret == HTTP_UPDATE_OK) {
        pm.info("done! restart...");
        ESP.restart();
    } else {
        pm.error("ret: " + getRetStr(ret));
    }
}
