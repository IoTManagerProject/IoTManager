#include "Updater.h"

#include "Config.h"
#include "WebClient.h"
#ifdef ESP8266
#include "ESP8266.h"
#else
#include "ESP32.h"
#endif
#include "PrintMessage.h"

namespace Updater {

#ifdef ESP8266
static const char* TAG_MCU = "esp8266";
#else
static const char* TAG_MCU = "esp32";
#endif
static const char* TAG_METADATA = "version.txt";
static const char* TAG_FIRMWARE_BIN = "firmware.bin";
static const char* TAG_FS_IMAGE = "fs.bin";

static const char* MODULE = "Update";

const String buildUpdateUrl(const char* file) {
    String buf = config.general()->getUpdateUrl();
    if (!buf.endsWith("/")) {
        buf += "/";
    }
    buf += TAG_MCU;
    buf += "/";
    buf += file;
    return buf;
}

const String check() {
    String url = buildUpdateUrl(TAG_METADATA);
    pm.info(url);
    String res = WebClient::get(url);
    pm.info(res);
    return res;
}

bool upgrade_fs_image() {
    pm.info(TAG_FS_IMAGE);
    WiFiClient wifiClient;
#ifdef ESP8266
    ESPhttpUpdate.rebootOnUpdate(false);
    HTTPUpdateResult ret = ESPhttpUpdate.updateSpiffs(wifiClient, buildUpdateUrl(TAG_FS_IMAGE));
#else
    httpUpdate.rebootOnUpdate(false);
    HTTPUpdateResult ret = httpUpdate.updateSpiffs(wifiClient, buildUpdateUrl(TAG_FS_IMAGE));
#endif
    if (ret != HTTP_UPDATE_OK) {
        pm.error(TAG_FS_IMAGE);
    }
    return ret == HTTP_UPDATE_OK;
}  // namespace Updater

bool upgrade_firmware() {
    pm.info(TAG_FIRMWARE_BIN);
    WiFiClient wifiClient;
#ifdef ESP8266
    HTTPUpdateResult ret = ESPhttpUpdate.update(wifiClient, buildUpdateUrl(TAG_FIRMWARE_BIN));
#else
    HTTPUpdateResult ret = httpUpdate.update(wifiClient, buildUpdateUrl(TAG_FIRMWARE_BIN));
#endif
    if (ret != HTTP_UPDATE_OK) {
        pm.error(TAG_FIRMWARE_BIN);
    }
    return (ret == HTTP_UPDATE_OK);
}

}  // namespace Updater