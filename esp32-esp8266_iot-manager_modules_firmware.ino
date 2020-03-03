#include "set.h"

void setup() {

  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  Serial.println("--------------started----------------");
  //--------------------------------------------------------------
  SPIFFS.begin();
  configSetup = readFile("config.json", 4096);
  Serial.println(configSetup);
  jsonWrite(configJson, "SSDP", jsonRead(configSetup, "SSDP"));
  jsonWrite(configJson, "lang", jsonRead(configSetup, "lang"));
  Serial.println("SPIFFS_init");

#ifdef ESP32
  uint32_t chipID_u = ESP.getEfuseMac();
  chipID = String(chipID_u);
  jsonWrite(configSetup, "chipID", chipID);
#endif

#ifdef ESP8266
  chipID = String( ESP.getChipId() ) + "-" + String( ESP.getFlashChipId());
  jsonWrite(configSetup, "chipID", chipID);
  Serial.setDebugOutput(0);
#endif

  jsonWrite(configSetup, "firmware_version", firmware_version);

  prex = prefix + "/" + chipID;
  Serial.println(chipID);
  //--------------------------------------------------------------
  CMD_init();
  Serial.println("[V] CMD_init");
  //--------------------------------------------------------------
  All_init();
  Serial.println("[V] All_init");
  //--------------------------------------------------------------
  WIFI_init();
  Serial.println("[V] WIFI_init");
  //--------------------------------------------------------------
  Web_server_init();
  Serial.println("[V] Web_server_init");
  //--------------------------------------------------------------
  Time_Init();
  Serial.println("[V] Time_Init");
  //--------------------------------------------------------------
  MQTT_init();
  Serial.println("[V] MQTT_init");
  //--------------------------------------------------------------
  Push_init();
  Serial.println("[V] Push_init");
  //--------------------------------------------------------------
  statistics();
  Serial.println("[V] statistics");
  //--------------------------------------------------------------
  initUpgrade();
  Serial.println("[V] initUpgrade");
  //--------------------------------------------------------------

  getMemoryLoad("[i] After loading");

  ts.add(TEST, 14400, [&](void*) {

    statistics();

  }, nullptr, false);
}

void loop() {

#ifdef OTA_enable
  ArduinoOTA.handle();
#endif

#ifdef WS_enable
  ws.cleanupClients();
#endif

  handleMQTT();

  handleCMD_loop();
  handleButton();
  handleScenario();

  ts.update();
  handle_upgrade();
}
