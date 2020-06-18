#include "Global.h"

static const char* UPGRADE_URL PROGMEM = "http://91.204.228.124:1100/update/";

const String getUpgradeUrl() {
    String url = FPSTR(UPGRADE_URL);
#ifdef ESP32
    url += "esp32";
#else
    url += "esp8266";
#endif
    url += "/version.txt";
    return url;
}

void initUpgrade() {
    String last_version = WiFi.status() == WL_CONNECTED ? getURL(getUpgradeUrl()) : "";
    jsonWriteStr(configSetup, "last_version", last_version);

    Serial.printf("[i] Last firmware version: %s\n", last_version.c_str());

    server.on("/check", HTTP_GET, [last_version](AsyncWebServerRequest* request) {
        upgrade_url = true;
        Serial.printf("[i] Last firmware version: %s\n", last_version.c_str());
        String tmp = "{}";
        if (WiFi.status() == WL_CONNECTED) {
            if (mb_4_of_memory) {
                if (last_version != "") {
                    if (last_version != "error") {
                        if (last_version == firmware_version) {
                            jsonWriteStr(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>Последняя версия прошивки уже установлена.");
                            jsonWriteStr(tmp, "class", "pop-up");
                        } else {
                            jsonWriteStr(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>Имеется новая версия прошивки<a href=\"#\" class=\"btn btn-block btn-danger\" onclick=\"send_request(this, '/upgrade');setTimeout(function(){ location.href='/'; }, 120000);html('my-block','<span class=loader></span>Идет обновление прошивки, после обновления страница  перезагрузится автоматически...')\">Установить</a>");
                            jsonWriteStr(tmp, "class", "pop-up");
                        }
                    } else {
                        jsonWriteStr(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>Ошибка... Cервер не найден. Попробуйте позже...");
                        jsonWriteStr(tmp, "class", "pop-up");
                    }
                } else {
                    jsonWriteStr(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>Нажмите на кнопку \"обновить прошивку\" повторно...");
                    jsonWriteStr(tmp, "class", "pop-up");
                }
            } else {
                jsonWriteStr(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>Обновление по воздуху не поддерживается, модуль имеет меньше 4 мб памяти...");
                jsonWriteStr(tmp, "class", "pop-up");
            }
        } else {
            jsonWriteStr(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>Устройство не подключен к роутеру...");
            jsonWriteStr(tmp, "class", "pop-up");
        }
        request->send(200, "text/text", tmp);
    });

    server.on("/upgrade", HTTP_GET, [](AsyncWebServerRequest* request) {
        upgrade = true;
        String tmp = "{}";
        request->send(200, "text/text", "ok");
    });
}

void do_upgrade_url() {
    if (upgrade_url) {
        upgrade_url = false;
#ifdef ESP32
        last_version = getURL("http://91.204.228.124:1100/update/esp32/version.txt");
        jsonWriteStr(configSetup, "last_version", last_version);
#endif
#ifdef ESP8266
        last_version = getURL("http://91.204.228.124:1100/update/esp8266/version.txt");
        jsonWriteStr(configSetup, "last_version", last_version);
#endif
    }
}

void upgrade_firmware() {
    String scenario_for_update;
    String config_for_update;
    String configSetup_for_update;
    scenario_for_update = readFile("firmware.s.txt", 4000);
    config_for_update = readFile("firmware.c.txt", 4000);
    configSetup_for_update = configSetup;

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

        Serial.println("SPIFFS upgrade done!");
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
            Serial.println("BUILD upgrade done!");
            Serial.println("Restart ESP....");
            ESP.restart();
        } else {
            Serial.println("!!!!BUILD upgrade ERROR");
        }
    } else {
        Serial.println("!!!!SPIFFS upgrade ERROR");
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

/*
  // ----------------------- Обновление с сайта
  void webUpgrade() {

  #ifdef ESP8266
  String spiffsData = "http://91.204.228.124:1100/update/esp8266/esp32-esp8266_iot-manager_modules_firmware.spiffs.bin";
  String buildData = "http://91.204.228.124:1100/update/esp8266/esp32-esp8266_iot-manager_modules_firmware.ino.bin";
  #endif

  #ifdef ESP32
  String spiffsData = "http://91.204.228.124:1100/update/esp32/esp32-esp8266_iot-manager_modules_firmware.spiffs.bin";
  String buildData = "http://91.204.228.124:1100/update/esp32/esp32-esp8266_iot-manager_modules_firmware.ino.bin";
  #endif

  if (spiffsData != "") { // Если нужно прошить FS
    String scenario_for_update;
    String config_for_update;
    String configSetup_for_update;
    Serial.println(spiffsData);
    scenario_for_update = readFile("firmware.s.txt", 2048);
    config_for_update = readFile("config.all.txt", 2048);
    configSetup_for_update = configSetup;
    ESPhttpUpdate.rebootOnUpdate(false); // Отключим перезагрузку после обновления
    updateHTTP(spiffsData, true);
    writeFile("firmware.s.txt", scenario_for_update);
    writeFile("config.all.txt", config_for_update);
    writeFile("config.json", configSetup_for_update);
    saveConfig();
  }

  if (buildData != "") { // Если нужно прошить build
    Serial.println(buildData);
    ESPhttpUpdate.rebootOnUpdate(true); // Включим перезагрузку после обновления
    updateHTTP(buildData, false);
  }
  }
  // ------------------ Обновление по url
  void updateHTTP(String url, boolean mode) {
  if (url == "") return;
  ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
  if (mode) {
    Serial.println("Update Spiffs...");
    t_httpUpdate_return ret = ESPhttpUpdate.updateSpiffs(url);
    UpdateStatus(ret , "Spiffs");
  } else {
    Serial.println("Update Build...");
    t_httpUpdate_return ret = ESPhttpUpdate.update(url);
    UpdateStatus(ret , "build");
  }
  }
  void UpdateStatus(t_httpUpdate_return set, String mode) {

  switch (set) {

    case HTTP_UPDATE_FAILED:
      Serial.println(mode + "_FAILED");
      var = "{}";
      jsonWriteStr(var, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>_FAILED");
      jsonWriteStr(var, "class", "pop-up");
      //request->send(200, "text/text", var);
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println(mode + "_NO_UPDATES");
      var = "{}";
      jsonWriteStr(var, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>_NO_UPDATES");
      jsonWriteStr(var, "class", "pop-up");
      //request->send(200, "text/text", var);
      break;

    case HTTP_UPDATE_OK:
      Serial.println(mode + "_UPDATE_OK");
      var = "{}";
      jsonWriteStr(var, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>_UPDATE_OK");
      jsonWriteStr(var, "class", "pop-up");
      //request->send(200, "text/text", var);
      break;
  }
  }
*/
