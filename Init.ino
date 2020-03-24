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

  ts.remove(ANALOG_);
  ts.remove(LEVEL);
  ts.remove(DALLAS);
  ts.remove(DHTT);
  ts.remove(DHTH);
  ts.remove(DHTC);
  ts.remove(DHTP);
  ts.remove(DHTD);
  ts.remove(STEPPER1);
  ts.remove(STEPPER2);

  all_widgets = "";
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
    String tmp = "";
    request->redirect("/page.htm?configuration");
  });

  server.on("/relay_timer", HTTP_GET, [](AsyncWebServerRequest * request) {
    writeFile("firmware.config.txt", readFile("configs/relay_t.config.txt", 2048));
    writeFile("firmware.scenario.txt", readFile("configs/relay_t.scenario.txt", 2048));
    Device_init();
    Scenario_init();
    request->redirect("/page.htm?configuration");
  });

  server.on("/relay_countdown", HTTP_GET, [](AsyncWebServerRequest * request) {
    writeFile("firmware.config.txt", readFile("configs/relay_c.config.txt", 2048));
    writeFile("firmware.scenario.txt", readFile("configs/relay_c.scenario.txt", 2048));
    Device_init();
    Scenario_init();
    request->redirect("/page.htm?configuration");
  });

  server.on("/relay_several", HTTP_GET, [](AsyncWebServerRequest * request) {
    writeFile("firmware.config.txt", readFile("configs/relay_s.config.txt", 2048));
    writeFile("firmware.scenario.txt", readFile("configs/relay_s.scenario.txt", 2048));
    Device_init();
    Scenario_init();
    request->redirect("/page.htm?configuration");
  });

  server.on("/relay_switch", HTTP_GET, [](AsyncWebServerRequest * request) {
    writeFile("firmware.config.txt", readFile("configs/relay_sw.config.txt", 2048));
    writeFile("firmware.scenario.txt", readFile("configs/relay_sw.scenario.txt", 2048));
    Device_init();
    Scenario_init();
    request->redirect("/page.htm?configuration");
  });

  server.on("/relay_button_remote", HTTP_GET, [](AsyncWebServerRequest * request) {
    writeFile("firmware.config.txt", readFile("configs/relay_br.config.txt", 2048));
    writeFile("firmware.scenario.txt", readFile("configs/relay_br.scenario.txt", 2048));
    Device_init();
    Scenario_init();
    request->redirect("/page.htm?configuration");
  });

  server.on("/relay_switch_remote", HTTP_GET, [](AsyncWebServerRequest * request) {
    writeFile("firmware.config.txt", readFile("configs/relay_sr.config.txt", 2048));
    writeFile("firmware.scenario.txt", readFile("configs/relay_sr.scenario.txt", 2048));
    Device_init();
    Scenario_init();
    request->redirect("/page.htm?configuration");
  });

  server.on("/pwm", HTTP_GET, [](AsyncWebServerRequest * request) {
    writeFile("firmware.config.txt", readFile("configs/pwm.config.txt", 2048));
    writeFile("firmware.scenario.txt", readFile("configs/pwm.scenario.txt", 2048));
    Device_init();
    Scenario_init();
    request->redirect("/page.htm?configuration");
  });

  //part 2===============================================================================

  server.on("/dht11", HTTP_GET, [](AsyncWebServerRequest * request) {
    writeFile("firmware.config.txt", readFile("configs/dht11.config.txt", 2048));
    writeFile("firmware.scenario.txt", readFile("configs/dht11.scenario.txt", 2048));
    Device_init();
    Scenario_init();
    request->redirect("/page.htm?configuration");
  });

  server.on("/dht22", HTTP_GET, [](AsyncWebServerRequest * request) {
    writeFile("firmware.config.txt", readFile("configs/dht22.config.txt", 2048));
    writeFile("firmware.scenario.txt", readFile("configs/dht22.scenario.txt", 2048));
    Device_init();
    Scenario_init();
    request->redirect("/page.htm?configuration");
  });

  server.on("/analog", HTTP_GET, [](AsyncWebServerRequest * request) {
    writeFile("firmware.config.txt", readFile("configs/analog.config.txt", 2048));
    writeFile("firmware.scenario.txt", readFile("configs/analog.scenario.txt", 2048));
    Device_init();
    Scenario_init();
    request->redirect("/page.htm?configuration");
  });

  server.on("/dallas", HTTP_GET, [](AsyncWebServerRequest * request) {
    writeFile("firmware.config.txt", readFile("configs/dallas.config.txt", 2048));
    writeFile("firmware.scenario.txt", readFile("configs/dallas.scenario.txt", 2048));
    Device_init();
    Scenario_init();
    request->redirect("/page.htm?configuration");
  });

  server.on("/termostat", HTTP_GET, [](AsyncWebServerRequest * request) {
    writeFile("firmware.config.txt", readFile("configs/termostat.config.txt", 2048));
    writeFile("firmware.scenario.txt", readFile("configs/termostat.scenario.txt", 2048));
    Device_init();
    Scenario_init();
    request->redirect("/page.htm?configuration");
  });

  server.on("/level", HTTP_GET, [](AsyncWebServerRequest * request) {
    writeFile("firmware.config.txt", readFile("configs/level.config.txt", 2048));
    writeFile("firmware.scenario.txt", readFile("configs/level.scenario.txt", 2048));
    Device_init();
    Scenario_init();
    request->redirect("/page.htm?configuration");
  });

  server.on("/moution_relay", HTTP_GET, [](AsyncWebServerRequest * request) {
    writeFile("firmware.config.txt", readFile("configs/moution_r.config.txt", 2048));
    writeFile("firmware.scenario.txt", readFile("configs/moution_r.scenario.txt", 2048));
    Device_init();
    Scenario_init();
    request->redirect("/page.htm?configuration");
  });

  server.on("/moution_security", HTTP_GET, [](AsyncWebServerRequest * request) {
    writeFile("firmware.config.txt", readFile("configs/moution_s.config.txt", 2048));
    writeFile("firmware.scenario.txt", readFile("configs/moution_s.scenario.txt", 2048));
    Device_init();
    Scenario_init();
    request->redirect("/page.htm?configuration");
  });

  server.on("/stepper", HTTP_GET, [](AsyncWebServerRequest * request) {
    writeFile("firmware.config.txt", readFile("configs/stepper.config.txt", 2048));
    writeFile("firmware.scenario.txt", readFile("configs/stepper.scenario.txt", 2048));
    Device_init();
    Scenario_init();
    request->redirect("/page.htm?configuration");
  });

  //default===============================================================================

  server.on("/default", HTTP_GET, [](AsyncWebServerRequest * request) {
    writeFile("firmware.config.txt", readFile("configs/firmware.config.txt", 2048));
    writeFile("firmware.scenario.txt", readFile("configs/firmware.scenario.txt", 2048));
    Device_init();
    Scenario_init();
    request->redirect("/page.htm?configuration");
  });
}

void up_time() {
  uint32_t ss = millis() / 1000;
  uint32_t mm = ss / 60;
  uint32_t hh = mm / 60;
  uint32_t dd = hh / 24;

  String out = "";

  if (ss != 0) {
    out = "[i] uptime = " + String(ss) + " sec";
    jsonWrite(configJson, "uptime", String(ss) + " sec");
  }
  if (mm != 0) {
    out = "[i] uptime = " + String(mm) + " min";
    jsonWrite(configJson, "uptime", String(mm) + " min");
  }
  if (hh != 0) {
    out = "[i] uptime = " + String(hh) + " hours";
    jsonWrite(configJson, "uptime", String(hh) + " hours");
  }
  if (dd != 0) {
    out = "[i] uptime = " + String(dd) + " days";
    jsonWrite(configJson, "uptime", String(dd) + " days");
  }
  Serial.println(out + ", mqtt_lost_error: " + String(mqtt_lost_error) + ", wifi_lost_error: " + String(wifi_lost_error));
}

void statistics_init() {
  ts.add(STATISTICS, statistics_update, [&](void*) {

    statistics();

  }, nullptr, true);
}

void statistics() {
  String urls = "http://backup.privet.lv/visitors/?";
  //-----------------------------------------------------------------
  urls += WiFi.macAddress().c_str();
  urls += "&";
  //-----------------------------------------------------------------
#ifdef ESP8266
  urls += "iot-manager_esp8266";
#endif
#ifdef ESP32
  urls += "iot-manager_esp32";
#endif
  urls += "&";
  //-----------------------------------------------------------------
#ifdef ESP8266
  urls += ESP.getResetReason();
#endif
#ifdef ESP32
  urls += "Unknown";
#endif
  urls += "&";
  //-----------------------------------------------------------------
  urls += "firm version: " + firmware_version + " " + DATE_COMPILING + " " + TIME_COMPILING; 
  //-----------------------------------------------------------------
  String stat = getURL(urls);
  //Serial.println(stat);
}
