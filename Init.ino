void All_init() {
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

  int array_sz = sizeof(sensors_reading_map) / sizeof(sensors_reading_map[0]);

  for (int i = 0; i <= array_sz; i++) {
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
  if (jsonReadStr(configSetup, "scen") == "1") {
    scenario = readFile("firmware.s.txt", 2048);
  }
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
    urls += "ver: " + firmware_version;
    //-----------------------------------------------------------------
    String stat = getURL(urls);
    //Serial.println(stat);
  }
}
