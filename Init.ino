void All_init() {

  server.on("/all_modules_init", HTTP_GET, [](AsyncWebServerRequest * request) {
    Device_init();
    request->send(200, "text/text", "OK"); // отправляем ответ о выполнении
  });

  server.on("/scenario", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasArg("status")) {
      jsonWrite(configSetup, "scenario", request->getParam("status")->value());
    }
    saveConfig();
    Scenario_init();
    request->send(200, "text/text", "OK"); // отправляем ответ о выполнении
  });

  server.on("/cleanlog", HTTP_GET, [](AsyncWebServerRequest * request) {
    SPIFFS.remove("/log.analog.txt");
    SPIFFS.remove("/log.dallas.txt");
    SPIFFS.remove("/log.level.txt");
    SPIFFS.remove("/log.ph.txt");
    SPIFFS.remove("/log.txt");
    request->send(200, "text/text", "OK"); // отправляем ответ о выполнении
  });

  prsets_init();

  Device_init();
  Scenario_init();
  Timer_countdown_init();
}

void Device_init() {

  //SENSORS-SECTION
  ts.remove(ANALOG_);
  ts.remove(LEVEL);
  ts.remove(DALLAS);
  ts.remove(DHTT);
  ts.remove(DHTH);
  //================

  all_vigets = "";
  txtExecution("firmware.config.txt");
  //outcoming_date();
}
//-------------------------------сценарии-----------------------------------------------------

void Scenario_init() {
  if (jsonRead(configSetup, "scenario") == "1") {
    scenario = readFile("firmware.scenario.txt", 2048);
  }
}

void prsets_init() {
  //part 1===============================================================================
  server.on("/relay", HTTP_GET, [](AsyncWebServerRequest * request) {
    writeFile("firmware.config.txt", readFile("configs/relay.config.txt", 2048));
    writeFile("firmware.scenario.txt", readFile("configs/relay.scenario.txt", 2048));
    Device_init();
    Scenario_init();
    request->send(200, "text/text", "OK"); // отправляем ответ о выполнении
  });

  server.on("/relay_timer", HTTP_GET, [](AsyncWebServerRequest * request) {
    writeFile("firmware.config.txt", readFile("configs/relay_t.config.txt", 2048));
    writeFile("firmware.scenario.txt", readFile("configs/relay_t.scenario.txt", 2048));
    Device_init();
    Scenario_init();
    request->send(200, "text/text", "OK"); // отправляем ответ о выполнении
  });

  server.on("/relay_countdown", HTTP_GET, [](AsyncWebServerRequest * request) {
    writeFile("firmware.config.txt", readFile("configs/relay_c.config.txt", 2048));
    writeFile("firmware.scenario.txt", readFile("configs/relay_c.scenario.txt", 2048));
    Device_init();
    Scenario_init();
    request->send(200, "text/text", "OK"); // отправляем ответ о выполнении
  });

  server.on("/relay_several", HTTP_GET, [](AsyncWebServerRequest * request) {
    writeFile("firmware.config.txt", readFile("configs/relay_s.config.txt", 2048));
    writeFile("firmware.scenario.txt", readFile("configs/relay_s.scenario.txt", 2048));
    Device_init();
    Scenario_init();
    request->send(200, "text/text", "OK"); // отправляем ответ о выполнении
  });
  //part 2===============================================================================
  server.on("/termostat", HTTP_GET, [](AsyncWebServerRequest * request) {
    writeFile("firmware.config.txt", readFile("configs/termostat.config.txt", 2048));
    writeFile("firmware.scenario.txt", readFile("configs/termostat.scenario.txt", 2048));
    Device_init();
    Scenario_init();
    request->send(200, "text/text", "OK"); // отправляем ответ о выполнении
  });

  server.on("/dht", HTTP_GET, [](AsyncWebServerRequest * request) {
    writeFile("firmware.config.txt", readFile("configs/dht.config.txt", 2048));
    writeFile("firmware.scenario.txt", readFile("configs/dht.scenario.txt", 2048));
    Device_init();
    Scenario_init();
    request->send(200, "text/text", "OK"); // отправляем ответ о выполнении
  });

  server.on("/default", HTTP_GET, [](AsyncWebServerRequest * request) {
    writeFile("firmware.config.txt", readFile("configs/firmware.config.txt", 2048));
    writeFile("firmware.scenario.txt", readFile("configs/firmware.scenario.txt", 2048));
    Device_init();
    Scenario_init();
    request->send(200, "text/text", "OK"); // отправляем ответ о выполнении
  });
}

void up_time() {
  uint32_t ss = millis() / 1000;
  uint32_t mm = ss / 60;
  uint32_t hh = mm / 60;
  uint32_t dd = hh / 24;

  if (ss != 0) {
    Serial.println(String(ss) + " sec");
    jsonWrite(configJson, "uptime", String(ss) + " sec");
  }
  if (mm != 0) {
    Serial.println(String(mm) + " min");
    jsonWrite(configJson, "uptime", String(mm) + " min");
  }
  if (hh != 0) {
    Serial.println(String(hh) + " hours");
    jsonWrite(configJson, "uptime", String(hh) + " hours");
  }
  if (dd != 0) {
    Serial.println(String(dd) + " days");
    jsonWrite(configJson, "uptime", String(dd) + " days");
  }
}
