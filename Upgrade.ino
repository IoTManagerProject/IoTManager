void initUpgrade() {
  server.on("/upgrade", HTTP_GET, [](AsyncWebServerRequest * request) {

#ifdef ESP32
    new_version = getURL("http://91.204.228.124:1100/update/esp32/version.txt");
#endif
#ifdef ESP8266
    //new_version = getURL("http://91.204.228.124:1100/update/esp8266/version.txt");
#endif

    Serial.println("[i] Last firmware version: ");
    Serial.print(new_version);

    String tmp = "{}";

    if (new_version != "error") {
      if (new_version == firmware_version) {
        jsonWrite(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>Последняя версия прошивки уже установлена.");
        jsonWrite(tmp, "class", "pop-up");
      } else {
        upgrade_flag = true;
        jsonWrite(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>Идет обновление прошивки... После завершения устройство перезагрузится.");
        jsonWrite(tmp, "class", "pop-up");
      }
    } else {
      jsonWrite(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>Ошибка... Cервер не найден. Попробуйте позже...");
      jsonWrite(tmp, "class", "pop-up");
    }

    request->send(200, "text/text", tmp);
  });
}

void upgrade_firmware() {

  String scenario_for_update;
  String config_for_update;
  String configSetup_for_update;
  scenario_for_update = readFile("firmware.scenario.txt", 3072);
  config_for_update = readFile("firmware.config.txt", 3072);
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

    writeFile("firmware.scenario.txt", scenario_for_update);
    writeFile("firmware.config.txt", config_for_update);
    writeFile("config.json", configSetup_for_update);
    saveConfig();

    Serial.println("SPIFFS upgrade done!");
    Serial.println("Start upgrade BUILD, please wait...");

#ifdef ESP32
    httpUpdate.rebootOnUpdate(true);
    t_httpUpdate_return ret = httpUpdate.update(client_for_upgrade, "http://91.204.228.124:1100/update/esp32/esp32-esp8266_iot-manager_modules_firmware.ino.bin");
#endif
#ifdef ESP8266
    ESPhttpUpdate.rebootOnUpdate(true);
    t_httpUpdate_return ret = ESPhttpUpdate.update(client_for_upgrade, "http://91.204.228.124:1100/update/esp8266/esp32-esp8266_iot-manager_modules_firmware.ino.bin");
#endif

    Serial.println("BUILD upgrade done!");
    Serial.println("Restart ESP....");

  } else {
    //upgrade_status(t_httpUpdate_return ret);
  }
}

void handle_upgrade() {
  if (upgrade_flag) {
    upgrade_flag = false;
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
    scenario_for_update = readFile("firmware.scenario.txt", 2048);
    config_for_update = readFile("config.all.txt", 2048);
    configSetup_for_update = configSetup;
    ESPhttpUpdate.rebootOnUpdate(false); // Отключим перезагрузку после обновления
    updateHTTP(spiffsData, true);
    writeFile("firmware.scenario.txt", scenario_for_update);
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
      jsonWrite(var, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>_FAILED");
      jsonWrite(var, "class", "pop-up");
      //request->send(200, "text/text", var);
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println(mode + "_NO_UPDATES");
      var = "{}";
      jsonWrite(var, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>_NO_UPDATES");
      jsonWrite(var, "class", "pop-up");
      //request->send(200, "text/text", var);
      break;

    case HTTP_UPDATE_OK:
      Serial.println(mode + "_UPDATE_OK");
      var = "{}";
      jsonWrite(var, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>_UPDATE_OK");
      jsonWrite(var, "class", "pop-up");
      //request->send(200, "text/text", var);
      break;
  }
  }
*/
