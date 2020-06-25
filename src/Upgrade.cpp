#include "Upgrade.h"

#include "Global.h"

static const char* MODULE = "Upgr";

static const char* filesystem_image_url PROGMEM = "http://91.204.228.124:1100/update/%s/fs.bin";
static const char* available_url PROGMEM = "http://91.204.228.124:1100/update/%s/version.txt";

const String getAvailableUrl(const char* mcu) {
    char buf[128];
    sprintf_P(buf, available_url, mcu);
    return buf;
}

void getLastVersion() {
    if (upgrade_url) {
        upgrade_url = false;
        String url;
#ifdef ESP32
        url = getAvailableUrl("esp32");
#endif
#ifdef ESP8266
        url = getAvailableUrl("esp8266");
#endif
        lastVersion = getURL(url);
        jsonWriteStr(configSetupJson, "last_version", lastVersion);
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
    String scenario_for_update;
    String config_for_update;
    String configSetup_for_update;

    scenario_for_update = readFile(String(DEVICE_SCENARIO_FILE), 4000);
    config_for_update = readFile(String(DEVICE_CONFIG_FILE), 4000);
    configSetup_for_update = configSetupJson;

    Serial.println("Start upgrade SPIFFS, please wait...");

    WiFiClient client_for_upgrade;

#ifdef ESP32
    httpUpdate.rebootOnUpdate(false);
    t_httpUpdate_return ret = httpUpdate.updateSpiffs(client_for_upgrade, "http://91.204.228.124:1100/update/esp32/esp32-esp8266_iot-manager_modules_firmware.spiffs.bin");
#endif
#ifdef ESP8266
    ESPhttpUpdate.rebootOnUpdate(false);
    t_httpUpdate_return ret = ESPhttpUpdate.updateSpiffs(client_for_upgrade, "http://91.204.228.124:1100/update/esp8266/esp32-esp8266_iot-manager_modules_firmware.spiffs.bin");
#endif

    if (ret == HTTP_UPDATE_OK) {
        writeFile(String(DEVICE_SCENARIO_FILE), scenario_for_update);
        writeFile(String(DEVICE_CONFIG_FILE), config_for_update);
        writeFile("config.json", configSetup_for_update);
        saveConfig();

        Serial.println("Upgrade done!");
        Serial.println("Start upgrade BUILD, please wait...");

#ifdef ESP32
        //httpUpdate.rebootOnUpdate(true);
        t_httpUpdate_return ret = httpUpdate.update(client_for_upgrade, "http://91.204.228.124:1100/update/esp32/esp32-esp8266_iot-manager_modules_firmware.ino.bin");
#endif
#ifdef ESP8266
        //ESPhttpUpdate.rebootOnUpdate(true);
        t_httpUpdate_return ret = ESPhttpUpdate.update(client_for_upgrade, "http://91.204.228.124:1100/update/esp8266/esp32-esp8266_iot-manager_modules_firmware.ino.bin");
#endif

        if (ret == HTTP_UPDATE_OK) {
            Serial.println("Upgrade done!");
            Serial.println("Restart...");
            ESP.restart();
        } else {
            Serial.println("[E] on build");
        }
    } else {
        Serial.println("[E] on upgrade");
    }
}

void flashUpgrade() {
    if (upgrade) {
        upgrade = false;
        upgrade_firmware();
    }
}

/*
  void upgrade_status(t_httpUpdate_return set) {
  switch (set) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("UPDATE_FAILED Error (%d): %s", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("NO_UPDATES");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
  }
  }
*/