void File_system_init() {
  
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  Serial.println("--------------started----------------");
  //--------------------------------------------------------------
  SPIFFS.begin();
  configSetupJson = readFile("config.json", 4096);
  configSetupJson.replace(" ", "");
  configSetupJson.replace("\r\n", "");
  Serial.println(configSetupJson);
  //jsonWriteStr(configLiveJson, "name", jsonReadStr(configSetupJson, "name"));
  //jsonWriteStr(configLiveJson, "lang", jsonReadStr(configSetupJson, "lang"));
  

#ifdef ESP32
  uint32_t chipID_u = ESP.getEfuseMac();
  chipID = String(chipID_u);
  jsonWriteStr(configSetupJson, "chipID", chipID);
#endif

#ifdef ESP8266
  chipID = String( ESP.getChipId() ) + "-" + String(ESP.getFlashChipId());
  jsonWriteStr(configSetupJson, "chipID", chipID);
  Serial.setDebugOutput(0);
#endif

  jsonWriteStr(configSetupJson, "firmware_version", firmware_version);

  prex = jsonReadStr(configSetupJson, "mqttPrefix") + "/" + chipID;
  Serial.println(chipID);
  
}

void get_esp_info() {

  
}
