void WIFI_init() {

  // --------------------Получаем ssid password со страницы
  server.on("/ssid", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasArg("ssid")) {
      jsonWrite(configSetup, "ssid", request->getParam("ssid")->value());
    }
    if (request->hasArg("password")) {
      jsonWrite(configSetup, "password", request->getParam("password")->value());
    }
    saveConfig();                 // Функция сохранения данных во Flash
    request->send(200, "text/text", "OK"); // отправляем ответ о выполнении
  });
  // --------------------Получаем ssidAP passwordAP со страницы
  server.on("/ssidap", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasArg("ssidAP")) {
      jsonWrite(configSetup, "ssidAP", request->getParam("ssidAP")->value());
    }
    if (request->hasArg("passwordAP")) {
      jsonWrite(configSetup, "passwordAP", request->getParam("passwordAP")->value());
    }
    saveConfig();                 // Функция сохранения данных во Flash
    request->send(200, "text/text", "OK"); // отправляем ответ о выполнении
  });


  // Попытка подключения к точке доступа

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
  }
  // Делаем проверку подключения до тех пор пока счетчик tries
  // не станет равен нулю или не получим подключение
  while (--tries && WiFi.status() != WL_CONNECTED)
  {
    if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.println("[E] password is not correct");
      tries = 1;
      jsonWrite(optionJson, "pass_status", 1);
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
    Serial.println("[V] IP address: ");
    Serial.println(WiFi.localIP());
    jsonWrite(configJson, "ip", WiFi.localIP().toString());

  }
}

bool StartAPMode() {
  /*
    Serial.println("WiFi up AP");
    IPAddress apIP(192, 168, 4, 1);
    IPAddress staticGateway(192, 168, 4, 1);
    IPAddress staticSubnet(255, 255, 255, 0);
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, staticGateway, staticSubnet);
    String _ssidAP = jsonRead(configSetup, "ssidAP");
    String _passwordAP = jsonRead(configSetup, "passwordAP");
    WiFi.softAP(_ssidAP.c_str(), _passwordAP.c_str());
    jsonWrite(configJson, "ip", apIP.toString());
  */
  Serial.println("WiFi up AP");
  WiFi.disconnect();

  WiFi.mode(WIFI_AP);

  String _ssidAP = jsonRead(configSetup, "ssidAP");
  String _passwordAP = jsonRead(configSetup, "passwordAP");
  WiFi.softAP(_ssidAP.c_str(), _passwordAP.c_str());
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  if (jsonReadtoInt(optionJson, "pass_status") != 1) {
    ts.add(ROUTER_SEARCHING, 30 * 1000, [&](void*) {
      Serial.println("->try find router");
      if (RouterFind(jsonRead(configSetup, "ssid"))) {
        ts.remove(ROUTER_SEARCHING);
        WIFI_init();
        MQTT_init();
      }
    }, nullptr, true);
  }
  return true;
}


boolean RouterFind(String ssid) {
  int n = WiFi.scanComplete ();
  if (n == -2) {                       //Сканирование не было запущено, запускаем
    WiFi.scanNetworks (true, false);   //async, show_hidden
    return false;
  }
  if (n == -1) {                       //Сканирование все еще выполняется
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
        Serial.print(ssid);
        Serial.print("<=>");
        Serial.println(WiFi.SSID(i));
      }
    }
    WiFi.scanDelete();
    return false;
  }
}


/*
  boolean RouterFind(String ssid) {

  int n = WiFi.scanComplete();

  Serial.print("status=");
  Serial.println(n);

  if (n == -2) {                       //Сканирование не было запущено, запускаем
    Serial.println("->enter to scanning function");
    WiFi.mode(WIFI_AP);
    WiFi.scanNetworks (true, false, false, 5000);   //async, show_hidden
    Serial.println("->out of scanning function");
    return false;

  }
  if (n == -1) {                       //Сканирование все еще выполняется
    Serial.println("->scanning in progress");
    return false;

  }
  if (n > 0) {                         //Найдено несколько сетей
    for (int i = 0; i <= n; i++) {
      if (WiFi.SSID(i) == ssid) {
        Serial.println("router found");
        WiFi.scanDelete();
        return true;
      } else {
        Serial.print(i);
        Serial.print(")");
        Serial.print(ssid);
        Serial.print("<=>");
        Serial.println(WiFi.SSID(i));
      }
    }
    WiFi.scanDelete();
    Serial.println("->scanning deleted");
    return false;
  }
  }

  void wifi_reset() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true, true);
  }
*/
