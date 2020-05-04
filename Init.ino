void All_init() {

  server.on("/init", HTTP_GET, [](AsyncWebServerRequest * request) {
    String value;
    if (request->hasArg("arg")) {
      value = request->getParam("arg")->value();
    }
    if (value == "0") {  //выкл сценариев
      jsonWriteStr(configSetup, "scenario", value);
      saveConfig();
      Scenario_init();
      request->send(200, "text/text", "OK");
    }
    if (value == "1") {   //вкл сценариев
      jsonWriteStr(configSetup, "scenario", value);
      saveConfig();
      Scenario_init();
      request->send(200, "text/text", "OK");
    }
    if (value == "2") {    //инициализация
      Device_init();
      request->send(200, "text/text", "OK");
    }
    if (value == "3") {
      clean_log_date();
      request->send(200, "text/text", "OK");
    }
    if (value == "4") {
      Scenario_init();
      request->send(200, "text/text", "OK");
    }
    if (value == "5") {
      i2c_scanning = true;
      request->redirect("/?utilities");
    }
  });

  prsets_init();
  Device_init();
  Scenario_init();
  Timer_countdown_init();
}

void Device_init() {

  logging_value_names_list = "";
  enter_to_logging_counter = LOG1 - 1;

  analog_value_names_list = "";
  enter_to_analog_counter = 0; 

  level_value_name = "";

  dhtT_value_name = "";
  dhtH_value_name = "";

  bmp280T_value_name = "";
  bmp280P_value_name = "";

  bme280T_value_name = "";
  bme280P_value_name = "";
  bme280H_value_name = "";
  bme280A_value_name = "";

  //int array_sz = sizeof(sensors_reading_map) / sizeof(sensors_reading_map[0]);

  for (int i = 0; i <= 14; i++) {
    sensors_reading_map[i] = 0;
  }

  for (int i = LOG1; i <= LOG5; i++) {
    ts.remove(i);
  }

#ifdef layout_in_ram
  all_widgets = "";
#else
  SPIFFS.remove("/layout.txt");
#endif

  txtExecution("firmware.c.txt");
  //outcoming_date();
}
//-------------------------------сценарии-----------------------------------------------------

void Scenario_init() {
  if (jsonRead(configSetup, "scenario") == "1") {
    scenario = readFile("firmware.s.txt", 2048);
  }
}

void prsets_init() {
  server.on("/preset", HTTP_GET, [](AsyncWebServerRequest * request) {
    String value;
    if (request->hasArg("arg")) {
      value = request->getParam("arg")->value();
    }
    if (value == "1") {
      writeFile("firmware.c.txt", readFile("configs/relay.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/relay.s.txt", 2048));
    }
    if (value == "2") {
      writeFile("firmware.c.txt", readFile("configs/relay_t.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/relay_t.s.txt", 2048));
    }
    if (value == "3") {
      writeFile("firmware.c.txt", readFile("configs/relay_c.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/relay_c.s.txt", 2048));
    }
    if (value == "4") {
      writeFile("firmware.c.txt", readFile("configs/relay_s.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/relay_s.s.txt", 2048));
    }
    if (value == "5") {
      writeFile("firmware.c.txt", readFile("configs/relay_sw.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/relay_sw.s.txt", 2048));
    }
    if (value == "6") {
      writeFile("firmware.c.txt", readFile("configs/relay_br.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/relay_br.s.txt", 2048));
    }
    if (value == "7") {
      writeFile("firmware.c.txt", readFile("configs/relay_sr.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/relay_sr.s.txt", 2048));
    }
    if (value == "8") {
      writeFile("firmware.c.txt", readFile("configs/pwm.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/pwm.s.txt", 2048));
    }
    if (value == "9") {
      writeFile("firmware.c.txt", readFile("configs/dht11.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/dht11.s.txt", 2048));
    }
    if (value == "10") {
      writeFile("firmware.c.txt", readFile("configs/dht22.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/dht22.s.txt", 2048));
    }
    if (value == "11") {
      writeFile("firmware.c.txt", readFile("configs/analog.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/analog.s.txt", 2048));
    }
    if (value == "12") {
      writeFile("firmware.c.txt", readFile("configs/dallas.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/dallas.s.txt", 2048));
    }
    if (value == "13") {
      writeFile("firmware.c.txt", readFile("configs/termostat.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/termostat.s.txt", 2048));
    }
    if (value == "14") {
      writeFile("firmware.c.txt", readFile("configs/level.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/level.s.txt", 2048));
    }
    if (value == "15") {
      writeFile("firmware.c.txt", readFile("configs/moution_r.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/moution_r.s.txt", 2048));
    }
    if (value == "16") {
      writeFile("firmware.c.txt", readFile("configs/moution_s.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/moution_s.s.txt", 2048));
    }
    if (value == "17") {
      writeFile("firmware.c.txt", readFile("configs/stepper.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/stepper.s.txt", 2048));
    }
    if (value == "18") {
      writeFile("firmware.c.txt", readFile("configs/servo.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/servo.s.txt", 2048));
    }
    if (value == "19") {
      writeFile("firmware.c.txt", readFile("configs/firmware.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/firmware.s.txt", 2048));
    }
    Device_init();
    Scenario_init();
    request->redirect("/?configuration");
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
    jsonWriteStr(configJson, "uptime", String(ss) + " sec");
  }
  if (mm != 0) {
    out = "[i] uptime = " + String(mm) + " min";
    jsonWriteStr(configJson, "uptime", String(mm) + " min");
  }
  if (hh != 0) {
    out = "[i] uptime = " + String(hh) + " hours";
    jsonWriteStr(configJson, "uptime", String(hh) + " hours");
  }
  if (dd != 0) {
    out = "[i] uptime = " + String(dd) + " days";
    jsonWriteStr(configJson, "uptime", String(dd) + " days");
  }
  Serial.println(out + ", mqtt_lost_error: " + String(mqtt_lost_error) + ", wifi_lost_error: " + String(wifi_lost_error));
}

void statistics_init() {
  ts.add(STATISTICS, statistics_update, [&](void*) {

    statistics();

  }, nullptr, true);
}

void statistics() {
  if (WiFi.status() == WL_CONNECTED) {
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
    //Serial.println(ESP.getResetReason());
#endif
#ifdef ESP32
    urls += "Power on";
#endif
    urls += "&";
    //-----------------------------------------------------------------
    urls += "ver: " + firmware_version; // + " " + DATE_COMPILING + " " + TIME_COMPILING;
    //-----------------------------------------------------------------
    String stat = getURL(urls);
    //Serial.println(stat);
  }
}
