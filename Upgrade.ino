void initUpgrade() {

  server.on("/upgrade", HTTP_GET, [](AsyncWebServerRequest * request) {

    Serial.println("start update...");

    //WiFiClient client_for;

    //httpUpdate.rebootOnUpdate(false);

    //t_httpUpdate_return ret = httpUpdate.updateSpiffs(client_for, "http://91.204.228.124:1100/update/esp32/esp32-esp8266_iot-manager_modules_firmware.spiffs.bin");

    //if (ret == HTTP_UPDATE_OK) {

    Serial.println("update Spiffs done!");

    //t_httpUpdate_return ret = httpUpdate.update(client_for, "http://91.204.228.124:1100/update/esp32/esp32-esp8266_iot-manager_modules_firmware.ino.bin");

    //       switch (ret) {
    //        case HTTP_UPDATE_FAILED:
    //          Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
    //         break;

    //       case HTTP_UPDATE_NO_UPDATES:
    //          Serial.println("HTTP_UPDATE_NO_UPDATES");
    //          break;

    //        case HTTP_UPDATE_OK:
    //          Serial.println("HTTP_UPDATE_OK");
    //          break;
    //      }

    // }

    //UpdateStatus(ret);

    /*    #ifdef ESP8266
        String new_version = getURL("http://91.204.228.124:1100/update/esp8266/version.txt");
        #endif

        #ifdef ESP32
        String new_version = getURL("http://91.204.228.124:1100/update/esp32/version.txt");
        #endif

        Serial.println(new_version);

        String tmp = "{}";
        if (new_version == firmware_version) {
          jsonWrite(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button> Установленна последняя версия: " + new_version);
          jsonWrite(tmp, "class", "pop-up");
        } else {
          jsonWrite(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button> Начитаню обновление до версии: " + new_version);
          jsonWrite(tmp, "class", "pop-up");
          webUpgrade();
        }*/

    request->send(200, "text/text", "ok");
  });

}

void UpdateStatus(t_httpUpdate_return set) {
  switch (set) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("HTTP_UPDATE_OK");
      break;
  }
}
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
