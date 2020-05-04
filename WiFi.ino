void WIFI_init() {

  // --------------------Получаем ssid password со страницы
  server.on("/ssid", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasArg("ssid")) {
      jsonWriteStr(configSetup, "ssid", request->getParam("ssid")->value());
    }
    if (request->hasArg("password")) {
      jsonWriteStr(configSetup, "password", request->getParam("password")->value());
    }
    saveConfig();                 // Функция сохранения данных во Flash
    request->send(200, "text/text", "OK"); // отправляем ответ о выполнении
  });
  // --------------------Получаем ssidAP passwordAP со страницы
  server.on("/ssidap", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasArg("ssidAP")) {
      jsonWriteStr(configSetup, "ssidAP", request->getParam("ssidAP")->value());
    }
    if (request->hasArg("passwordAP")) {
      jsonWriteStr(configSetup, "passwordAP", request->getParam("passwordAP")->value());
    }
    saveConfig();                 // Функция сохранения данных во Flash
    request->send(200, "text/text", "OK"); // отправляем ответ о выполнении
  });

  // --------------------Получаем логин и пароль для web со страницы
  server.on("/web", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasArg("web_login")) {
      jsonWriteStr(configSetup, "web_login", request->getParam("web_login")->value());
    }
    if (request->hasArg("web_pass")) {
      jsonWriteStr(configSetup, "web_pass", request->getParam("web_pass")->value());
    }
    saveConfig();                 // Функция сохранения данных во Flash
    //Web_server_init();
    request->send(200, "text/text", "OK"); // отправляем ответ о выполнении
  });

  server.on("/restart", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasArg("device")) {
      if (request->getParam("device")->value() == "ok") ESP.restart();
    }
    request->send(200, "text/text", "OK"); // отправляем ответ о выполнении
  });
  ROUTER_Connecting();
}

void ROUTER_Connecting() {

  WiFi.mode(WIFI_STA);

  byte tries = 20;
  String _ssid = jsonRead(configSetup, "ssid");
  String _password = jsonRead(configSetup, "password");
  //WiFi.persistent(false);

  if (_ssid == "" && _password == "") {
    WiFi.begin();
  }
  else {
    WiFi.begin(_ssid.c_str(), _password.c_str());
    Serial.print("ssid: ");
    Serial.println(_ssid);
  }
  // Делаем проверку подключения до тех пор пока счетчик tries
  // не станет равен нулю или не получим подключение
  while (--tries && WiFi.status() != WL_CONNECTED)
  {
    if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.println("[E] password is not correct");
      tries = 1;
      jsonWriteInt(optionJson, "pass_status", 1);
    }
    Serial.print(".");
    delay(1000);
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    // Если не удалось подключиться запускаем в режиме AP
    Serial.println("");
    // WiFi.disconnect(true);
    StartAPMode();

  }
  else {
    // Иначе удалось подключиться отправляем сообщение
    // о подключении и выводим адрес IP
    Serial.println("");
    Serial.println("[V] WiFi connected");
    Serial.print("[V] IP address: http://");
    Serial.print(WiFi.localIP());
    Serial.println("");
    jsonWriteStr(configJson, "ip", WiFi.localIP().toString());
    
    //add_dev_in_list("dev.txt", chipID, WiFi.localIP().toString());

  }
}

bool StartAPMode() {
  Serial.println("WiFi up AP");
  WiFi.disconnect();

  WiFi.mode(WIFI_AP);

  String _ssidAP = jsonRead(configSetup, "ssidAP");
  String _passwordAP = jsonRead(configSetup, "passwordAP");
  WiFi.softAP(_ssidAP.c_str(), _passwordAP.c_str());
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  jsonWriteStr(configJson, "ip", myIP.toString());

  if (jsonReadtoInt(optionJson, "pass_status") != 1) {
    ts.add(ROUTER_SEARCHING, 10 * 1000, [&](void*) {
      Serial.println("->try find router");
      if (RouterFind(jsonRead(configSetup, "ssid"))) {
        ts.remove(ROUTER_SEARCHING);
        WiFi.scanDelete();
        ROUTER_Connecting();
        MQTT_init();
      }
    }, nullptr, true);
  }
  return true;
}


boolean RouterFind(String ssid) {
  int n = WiFi.scanComplete ();
  Serial.println("n = " + String(n));
  if (n == -2) {                       //Сканирование не было запущено, запускаем
    Serial.println("[WIFI][i] scanning has not been triggered, starting scanning");
    WiFi.scanNetworks (true, false);   //async, show_hidden
    return false;
  }
  if (n == -1) {                       //Сканирование все еще выполняется
    Serial.println("[WIFI][i] scanning still in progress");
    return false;
  }
  if (n == 0) {                       //ни одна сеть не найдена
    Serial.println("[WIFI][i] no any wifi sations, starting scanning");
    WiFi.scanNetworks (true, false);
    return false;
  }
  if (n > 0) {
    for (int i = 0; i <= n; i++) {
      if (WiFi.SSID (i) == ssid) {
        WiFi.scanDelete();
        return true;
      } else {
        Serial.print(i);
        Serial.print(")");
        //Serial.print(ssid);
        //Serial.print("<=>");
        if (i == n) {
          Serial.print(WiFi.SSID(i));
          Serial.println("; ");
        } else {
          Serial.print(WiFi.SSID(i));
          Serial.println("; ");
        }
      }
    }
    WiFi.scanDelete();
    return false;
  }
}

/*
  String scanWIFI() {
  uint8_t n = WiFi.scanNetworks();
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  JsonArray& networks = json.createNestedArray("networks");
  for (uint8_t i = 0; i < n; i++) {
    JsonObject& data = networks.createNestedObject();
    String ssidMy = WiFi.SSID(i);
    data["ssid"] = ssidMy;
    data["pass"] = (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? "" : "*";
    int8_t dbm = WiFi.RSSI(i);
    data["dbm"] = dbm;
    if (ssidMy == jsonRead(configSetup, "ssid")) {
      jsonWriteStr(configJson, "dbm", dbm);
    }
  }
  String root;
  json.printTo(root);
  return root;
  }
*/
/*
  {
   "type":"wifi",
   "title":"{{LangWiFi1}}",
   "name":"ssid",
   "state":"{{ssid}}",
   "pattern":".{1,}"
  },
  {
   "type":"password",
   "title":"{{LangPass}}",
   "name":"ssidPass",
   "state":"{{ssidPass}}",
   "pattern":".{8,}"
  },
*/
