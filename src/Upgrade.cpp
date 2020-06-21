#include "Global.h"
void initUpgrade() {
#ifdef ESP8266
    if (WiFi.status() == WL_CONNECTED) last_version = getURL("http://91.204.228.124:1100/update/esp8266/version.txt");
#endif
#ifdef ESP32
    if (WiFi.status() == WL_CONNECTED) last_version = getURL("http://91.204.228.124:1100/update/esp32/version.txt");
#endif
    jsonWriteStr(configSetupJson, "last_version", last_version);
    Serial.print("[I] Last firmware version: ");
    Serial.println(last_version);
}

void do_upgrade_url() {
    if (upgrade_url) {
        upgrade_url = false;
#ifdef ESP32
        last_version = getURL("http://91.204.228.124:1100/update/esp32/version.txt");
#endif
#ifdef ESP8266
        last_version = getURL("http://91.204.228.124:1100/update/esp8266/version.txt");
#endif
        jsonWriteStr(configSetupJson, "last_version", last_version);
    }
}

void upgrade_firmware() {
    String scenario_for_update;
    String config_for_update;
    String configSetup_for_update;
    scenario_for_update = readFile("firmware.s.txt", 4000);
    config_for_update = readFile("firmware.c.txt", 4000);
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
        writeFile("firmware.s.txt", scenario_for_update);
        writeFile("firmware.c.txt", config_for_update);
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

void do_upgrade() {
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