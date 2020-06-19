#include "Global.h"

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

  levelPr_value_name = "";
  ultrasonicCm_value_name = "";

  dhtT_value_name = "";
  dhtH_value_name = "";

  bmp280T_value_name = "";
  bmp280P_value_name = "";

  bme280T_value_name = "";
  bme280P_value_name = "";
  bme280H_value_name = "";
  bme280A_value_name = "";

  int array_sz = sizeof(sensors_reading_map) / sizeof(sensors_reading_map[0]);

  for (int i = 0; i < array_sz; i++) {
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
  if (jsonReadStr(configSetupJson, "scen") == "1") {
    scenario = readFile("firmware.s.txt", 2048);
  }
}

void uptime_init() {
  ts.add(UPTIME, 5000, [&](void*) {
    handle_uptime();
  }, nullptr, true);
  ts.add(STATISTICS, statistics_update, [&](void*) {
    handle_statistics();
  }, nullptr, true);
}

void handle_uptime() {
  if (myUpTime.check()) {
    jsonWriteStr(configSetupJson, "uptime", uptime_as_string());
  }
}

void handle_statistics() {
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
    urls += "ver: ";
    urls += String(firmware_version);
    //-----------------------------------------------------------------
    String stat = getURL(urls);
    //Serial.println(stat);
  }
}