#include "set.h"

void setup() {
  //--------------------------------------------------------------
  File_system_init();
  Serial.println("SPIFFS_init");
  //--------------------------------------------------------------
  CMD_init();
  Serial.println("[V] CMD_init");
  //--------------------------------------------------------------
  sensors_init();
  Serial.println("[V] sensors_init");
  //--------------------------------------------------------------
  All_init();
  Serial.println("[V] All_init");
  //--------------------------------------------------------------
  WIFI_init();
  Serial.println("[V] WIFI_init");
  //--------------------------------------------------------------
  statistics_init();
  Serial.println("[V] statistics_init");
  //--------------------------------------------------------------
  initUpgrade();
  Serial.println("[V] initUpgrade");
  //--------------------------------------------------------------
  Web_server_init();
  Serial.println("[V] Web_server_init");
  //--------------------------------------------------------------
  MQTT_init();
  Serial.println("[V] MQTT_init");
  //--------------------------------------------------------------
  Time_Init();
  Serial.println("[V] Time_Init");
  //--------------------------------------------------------------
#ifdef push_enable
  Push_init();
  Serial.println("[V] Push_init");
#endif
  //--------------------------------------------------------------
#ifdef UDP_enable
  UDP_init();
  Serial.println("[V] UDP_init");
#endif
  //--------------------------------------------------------------



  ts.add(TEST, 10000, [&](void*) {
    getMemoryLoad("[i] periodic check of");
    //ws.textAll(json);
  }, nullptr, true);


  just_load = false;
}


void loop() {

#ifdef OTA_enable
  ArduinoOTA.handle();
#endif

#ifdef WS_enable
  ws.cleanupClients();
#endif

  not_async_actions();

  handleMQTT();
  handleCMD_loop();
  handleButton();
  handleScenario();
#ifdef UDP_enable
  handleUdp();
#endif
  ts.update();
}

void not_async_actions() {
  do_mqtt_connection();
  do_upgrade_url();
  do_upgrade();
#ifdef UDP_enable
  do_udp_data_parse();
  do_mqtt_send_settings_to_udp();
#endif
  do_i2c_scanning();
}
