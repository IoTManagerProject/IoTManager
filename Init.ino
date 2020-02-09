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
  txtExecution("config.all.txt");
  //outcoming_date();
}
//-------------------------------сценарии-----------------------------------------------------

void Scenario_init() {
  if (jsonRead(configSetup, "scenario") == "1") {
    scenario = readFile(scenarioFileNameS, 2048);
  }
}
