#include "Updater.h"

#include "ESP8266.h"
#include "Utils/PrintMessage.h"
#include "WebClient.h"

namespace Updater {
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

const String buildUpdateUrl(const char* mcu, const char* file) {
    char buf[128];
    sprintf_P(buf, UPDATE_SERVER_URL, mcu, file);
    return buf;
}

const String check() {
    String url;
#ifdef ESP8266
    url = buildUpdateUrl(TAG_MCU, TAG_METADATA);
#else
    url = buildUpdateUrl(TAG_MCU, TAG_METADATA);
#endif
    return WebClient::get(url);
}

bool upgrade_fs_image() {
    pm.info(TAG_FS_IMAGE);
    WiFiClient wifiClient;
#ifdef ESP8266
    ESPhttpUpdate.rebootOnUpdate(false);
    HTTPUpdateResult ret = ESPhttpUpdate.updateSpiffs(wifiClient, buildUpdateUrl(TAG_MCU, TAG_FS_IMAGE));
#else
    httpUpdate.rebootOnUpdate(false);
    HTTPUpdateResult ret = httpUpdate.updateSpiffs(wifiClient, buildUpdateUrl(TAG_MCU, TAG_FS_IMAGE);
#endif
    return ret == HTTP_UPDATE_OK;
}

bool upgrade_firmware() {
    pm.info(TAG_FIRMWARE_BIN);
    WiFiClient wifiClient;
#ifdef ESP8266
    HTTPUpdateResult ret = ESPhttpUpdate.update(wifiClient, buildUpdateUrl(TAG_MCU, TAG_FIRMWARE_BIN));
#else
    ret = httpUpdate.update(wifiClient, buildUpdateUrl(TAG_MCU, TAG_FIRMWARE_BIN));
#endif
    return (ret == HTTP_UPDATE_OK);
}

}  // namespace Updater