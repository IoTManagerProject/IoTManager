void SSDP_init() {
  server.on("/ssdp", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasArg("ssdp")) {
      jsonWriteStr(configSetup, "SSDP", request->getParam("ssdp")->value());
      jsonWriteStr(configJson, "SSDP", request->getParam("ssdp")->value());
    }
    saveConfig();
    request->send(200, "text/text", "OK");
  });
}

/*
  // --------------------Получаем ssdp со страницы
  server.on("/ssdp", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasArg("ssdp")) {
      jsonWriteStr(configSetup, "SSDP", request->getParam("ssdp")->value());
      jsonWriteStr(configJson, "SSDP", request->getParam("ssdp")->value());
      SSDP.setName(jsonRead(configSetup, "SSDP"));
    }
    saveConfig();
    request->send(200, "text/text", "OK");
  });

  // SSDP дескриптор
  server.on("/description.xml", [](AsyncWebServerRequest * request) {
    //SSDP.schema(http.client());
    request->send(200, "text/text", "OK");
  });


  if (WiFi.status() == WL_CONNECTED) {
    //Если версия  2.0.0 закаментируйте следующую строчку
    SSDP.setDeviceType("upnp:rootdevice");
    SSDP.setSchemaURL("description.xml");
    SSDP.setHTTPPort(80);
    SSDP.setName(jsonRead(configSetup, "SSDP"));
    SSDP.setSerialNumber(chipID);
    SSDP.setURL("/");
    SSDP.setModelName("tech");
    SSDP.setModelNumber(chipID + "/" + jsonRead(configSetup, "SSDP"));


    SSDP.setModelURL("https://github.com/DmitryBorisenko33/esp32-esp8266_iot-manager_modules_firmware");
    SSDP.setManufacturer("Borisenko Dmitry");
    SSDP.setManufacturerURL("https://www.instagram.com/rriissee3");
    SSDP.begin();
  }
  }
*/
