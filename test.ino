/*
UPD 
String br = "\n\n";


void info_init(){
  server.on("/info", HTTP_GET, [](AsyncWebServerRequest * request) {
    String r = ESP_info_html();
    request->send(200, "text/html", "<html><head><meta charset=""utf-8""></head><body>"+r+"</body></html>"); // отправляем ответ о выполнении
  });
  sCmd.addCommand("pwr",  pwr);
}

String ESP_info_str(){
  br = "\n\n";
  return ESP_info();
}
String ESP_info_html(){
  br = "<br>";
  return ESP_info();
}

String ESP_info(){

  FlashMode_t ideMode = ESP.getFlashChipMode();
  String message = "hello from esp8266!"+br;
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;  
  message += "Uptime: "+String(hr)+":"+String(min % 60)+":"+String(sec % 60)+br;
  message += "Vcc: " + String(ESP.getVcc() / 1024.0)+br;
  message += "ESP Core Version: \t"+ ESP.getCoreVersion()+br;
  message += "ESP Sdk Version: \t"+ String(ESP.getSdkVersion())+br;
  message += "ESP Boot Mode: \t"+ String(ESP.getBootMode())+br;
  
  message += "ESP Sketch Size: \t"+ String(ESP.getSketchSize())+br;
  message += "ESP Free Sketch Space: \t"+ String(ESP.getFreeSketchSpace())+br;
  
#ifdef ESP32
  message += "wifi station get hostname:  \t"+ String(wifi_station_get_hostname())+br;
  message += "ESP min Sketch Space: \t"+ String(esp_get_minimum_free_heap_size())+br;
#endif
  message += "размер свободной памяти:\t"+String(ESP.getFreeHeap())+br;
  message += "chip ID:\t"+String(ESP.getChipId(),HEX)+br;
  message += "flash chip ID:\t"+String(ESP.getFlashChipId(),HEX)+br;
  
  message += "размер флеш памяти IDE:\t"+String(ESP.getFlashChipSize())+br;
  message += "размер флеш памяти SDK:\t"+String(ESP.getFlashChipRealSize())+br;
  
  message += "частота флеш памяти, в Гц.:\t"+String(ESP.getFlashChipSpeed())+br;
  message += "Flash ide mode:\t"+ String(ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN")+br;
//  message += "IP address: \t"+ WiFi.localIP()+br;
  return message;
}
*/
