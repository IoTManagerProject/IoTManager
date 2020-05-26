void File_system_init() {
  
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  Serial.println("--------------started----------------");
  //--------------------------------------------------------------
  SPIFFS.begin();
  configSetup = readFile("config.json", 4096);
  configSetup.replace(" ", "");
  configSetup.replace("\r\n", "");
  Serial.println(configSetup);
  jsonWriteStr(configJson, "name", jsonReadStr(configSetup, "name"));
  jsonWriteStr(configJson, "lang", jsonReadStr(configSetup, "lang"));
  

#ifdef ESP32
  uint32_t chipID_u = ESP.getEfuseMac();
  chipID = String(chipID_u);
  jsonWriteStr(configSetup, "chipID", chipID);
#endif

#ifdef ESP8266
  chipID = String( ESP.getChipId() ) + "-" + String(ESP.getFlashChipId());
  jsonWriteStr(configSetup, "chipID", chipID);
  Serial.setDebugOutput(0);
#endif

  jsonWriteStr(configSetup, "firmware_version", firmware_version);

  prex = jsonReadStr(configSetup, "mqttPrefix") + "/" + chipID;
  Serial.println(chipID);
  
}

void get_esp_info() {

  
}
