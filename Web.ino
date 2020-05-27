void web_init() {
  /******************************************************************************
  **********************************INITIALIZATION*******************************
  ******************************************************************************/
  server.on("/set", HTTP_GET, [](AsyncWebServerRequest * request) {
    String value;
    //============================device settings=====================================
    if (request->hasArg("devinit")) {
      Device_init();
      request->send(200, "text/text", "OK");
    }

    if (request->hasArg("scen")) {
      value = request->getParam("scen")->value();
      if (value == "0") {
        jsonWriteStr(configSetup, "scen", value);
        saveConfig();
        Scenario_init();
      }
      if (value == "1") {
        jsonWriteStr(configSetup, "scen", value);
        saveConfig();
        Scenario_init();
      }
      request->send(200, "text/text", "OK");
    }

    if (request->hasArg("sceninit")) {
      Scenario_init();
      request->send(200, "text/text", "OK");
    }

    if (request->hasArg("cleanlog")) {
#ifdef logging_enable
      clean_log_date();
#endif
    }
    //==============================udp settings=============================================
    if (request->hasArg("udponoff")) {
      value = request->getParam("udponoff")->value();
      if (value == "0") {
        jsonWriteStr(configSetup, "udponoff", value);
        saveConfig();
        Scenario_init();
      }
      if (value == "1") {
        jsonWriteStr(configSetup, "udponoff", value);
        saveConfig();
        Scenario_init();
      }
      request->send(200, "text/text", "OK");
    }

    if (request->hasArg("updatelist")) {
      SPIFFS.remove("/dev.csv");
      addFile("dev.csv", "device id;device name;ip address");
      request->redirect("/?set.udp");
    }

    if (request->hasArg("updatepage")) {
      request->redirect("/?set.udp");
    }




    /*if (value == "5") {
      i2c_scanning = true;
      request->redirect("/?utilities");
      }*/

    /*if (value == "2") {
      mqtt_send_settings_to_udp = true;
      request->send(200, "text/text", "ok");
      }*/
  });
  /******************************************************************************
  **********************************TIME*****************************************
  ******************************************************************************/
  server.on("/time", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasArg("timezone")) {
      jsonWriteStr(configSetup, "timezone", request->getParam("timezone")->value());
    }
    if (request->hasArg("ntp")) {
      jsonWriteStr(configSetup, "ntp", request->getParam("ntp")->value());
    }
    saveConfig();
    reconfigTime();
    request->send(200, "text/text", "OK"); // отправляем ответ о выполнении
  });
  /******************************************************************************
  **********************************UPDATE***************************************
  ******************************************************************************/
  server.on("/check", HTTP_GET, [](AsyncWebServerRequest * request) {
    upgrade_url = true;
    Serial.print("[i] Last firmware version: ");
    Serial.println(last_version);
    String tmp = "{}";
    if (WiFi.status() == WL_CONNECTED) {
      if (mb_4_of_memory) {
        if (last_version != "") {
          if (last_version != "error") {
            if (last_version == firmware_version) {
              jsonWriteStr(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>Последняя версия прошивки уже установлена.");
              jsonWriteStr(tmp, "class", "pop-up");
            } else {
              jsonWriteStr(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>Имеется новая версия прошивки<a href=\"#\" class=\"btn btn-block btn-danger\" onclick=\"send_request(this, '/upgrade');setTimeout(function(){ location.href='/'; }, 120000);html('my-block','<span class=loader></span>Идет обновление прошивки, после обновления страница  перезагрузится автоматически...')\">Установить</a>");
              jsonWriteStr(tmp, "class", "pop-up");
            }
          } else {
            jsonWriteStr(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>Ошибка... Cервер не найден. Попробуйте позже...");
            jsonWriteStr(tmp, "class", "pop-up");
          }
        } else {
          jsonWriteStr(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>Нажмите на кнопку \"обновить прошивку\" повторно...");
          jsonWriteStr(tmp, "class", "pop-up");
        }
      } else {
        jsonWriteStr(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>Обновление по воздуху не поддерживается, модуль имеет меньше 4 мб памяти...");
        jsonWriteStr(tmp, "class", "pop-up");
      }
    } else {
      jsonWriteStr(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>Устройство не подключен к роутеру...");
      jsonWriteStr(tmp, "class", "pop-up");
    }
    request->send(200, "text/text", tmp);
  });

  server.on("/upgrade", HTTP_GET, [](AsyncWebServerRequest * request) {
    upgrade = true;
    String tmp = "{}";
    request->send(200, "text/text", "ok");
  });
  /******************************************************************************
  **********************************WIFI*****************************************
  ******************************************************************************/
  server.on("/ssid", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasArg("ssid")) {
      jsonWriteStr(configSetup, "ssid", request->getParam("ssid")->value());
    }
    if (request->hasArg("password")) {
      jsonWriteStr(configSetup, "password", request->getParam("password")->value());
    }
    saveConfig();
    request->send(200, "text/text", "OK");
  });
  server.on("/ssidap", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasArg("ssidAP")) {
      jsonWriteStr(configSetup, "ssidAP", request->getParam("ssidAP")->value());
    }
    if (request->hasArg("passwordAP")) {
      jsonWriteStr(configSetup, "passwordAP", request->getParam("passwordAP")->value());
    }
    saveConfig();
    request->send(200, "text/text", "OK");
  });
  server.on("/web", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasArg("web_login")) {
      jsonWriteStr(configSetup, "web_login", request->getParam("web_login")->value());
    }
    if (request->hasArg("web_pass")) {
      jsonWriteStr(configSetup, "web_pass", request->getParam("web_pass")->value());
    }
    saveConfig();
    //Web_server_init();
    request->send(200, "text/text", "OK");
  });
  server.on("/restart", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasArg("device")) {
      if (request->getParam("device")->value() == "ok") ESP.restart();
    }
    request->send(200, "text/text", "OK");
  });
  /******************************************************************************
   **********************************MQTT****************************************
   ******************************************************************************/
  server.on("/mqttSave", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasArg("mqttServer")) {
      jsonWriteStr(configSetup, "mqttServer", request->getParam("mqttServer")->value());
    }
    if (request->hasArg("mqttPort")) {
      int port = (request->getParam("mqttPort")->value()).toInt();
      jsonWriteInt(configSetup, "mqttPort", port);
    }
    if (request->hasArg("mqttPrefix")) {
      jsonWriteStr(configSetup, "mqttPrefix", request->getParam("mqttPrefix")->value());
    }
    if (request->hasArg("mqttUser")) {
      jsonWriteStr(configSetup, "mqttUser", request->getParam("mqttUser")->value());
    }
    if (request->hasArg("mqttPass")) {
      jsonWriteStr(configSetup, "mqttPass", request->getParam("mqttPass")->value());
    }
    saveConfig();
    mqtt_connection = true;
    request->send(200, "text/text", "ok");
  });
  server.on("/mqttCheck", HTTP_GET, [](AsyncWebServerRequest * request) {
    String tmp = "{}";
    jsonWriteStr(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>" + stateMQTT());
    jsonWriteStr(tmp, "class", "pop-up");
    request->send(200, "text/text", tmp);
  });
  /******************************************************************************
  **********************************PUSH*****************************************
  ******************************************************************************/
#ifdef push_enable
  server.on("/pushingboxDate", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasArg("pushingbox_id")) {
      jsonWriteStr(configSetup, "pushingbox_id", request->getParam("pushingbox_id")->value());
    }
    saveConfig();
    request->send(200, "text/text", "ok");
  });
#endif
  /******************************************************************************
  **********************************PRESETS**************************************
  ******************************************************************************/
  server.on("/preset", HTTP_GET, [](AsyncWebServerRequest * request) {
    String value;
    if (request->hasArg("arg")) {
      value = request->getParam("arg")->value();
    }
    if (value == "1") {
      writeFile("firmware.c.txt", readFile("configs/relay.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/relay.s.txt", 2048));
    }
    if (value == "2") {
      writeFile("firmware.c.txt", readFile("configs/relay_t.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/relay_t.s.txt", 2048));
    }
    if (value == "3") {
      writeFile("firmware.c.txt", readFile("configs/relay_c.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/relay_c.s.txt", 2048));
    }
    if (value == "4") {
      writeFile("firmware.c.txt", readFile("configs/relay_s.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/relay_s.s.txt", 2048));
    }
    if (value == "5") {
      writeFile("firmware.c.txt", readFile("configs/relay_sw.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/relay_sw.s.txt", 2048));
    }
    if (value == "6") {
      writeFile("firmware.c.txt", readFile("configs/relay_br.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/relay_br.s.txt", 2048));
    }
    if (value == "7") {
      writeFile("firmware.c.txt", readFile("configs/relay_sr.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/relay_sr.s.txt", 2048));
    }
    if (value == "8") {
      writeFile("firmware.c.txt", readFile("configs/pwm.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/pwm.s.txt", 2048));
    }
    if (value == "9") {
      writeFile("firmware.c.txt", readFile("configs/dht11.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/dht11.s.txt", 2048));
    }
    if (value == "10") {
      writeFile("firmware.c.txt", readFile("configs/dht22.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/dht22.s.txt", 2048));
    }
    if (value == "11") {
      writeFile("firmware.c.txt", readFile("configs/analog.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/analog.s.txt", 2048));
    }
    if (value == "12") {
      writeFile("firmware.c.txt", readFile("configs/dallas.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/dallas.s.txt", 2048));
    }
    if (value == "13") {
      writeFile("firmware.c.txt", readFile("configs/termostat.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/termostat.s.txt", 2048));
    }
    if (value == "14") {
      writeFile("firmware.c.txt", readFile("configs/level.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/level.s.txt", 2048));
    }
    if (value == "15") {
      writeFile("firmware.c.txt", readFile("configs/moution_r.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/moution_r.s.txt", 2048));
    }
    if (value == "16") {
      writeFile("firmware.c.txt", readFile("configs/moution_s.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/moution_s.s.txt", 2048));
    }
    if (value == "17") {
      writeFile("firmware.c.txt", readFile("configs/stepper.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/stepper.s.txt", 2048));
    }
    if (value == "18") {
      writeFile("firmware.c.txt", readFile("configs/servo.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/servo.s.txt", 2048));
    }
    if (value == "19") {
      writeFile("firmware.c.txt", readFile("configs/firmware.c.txt", 2048));
      writeFile("firmware.s.txt", readFile("configs/firmware.s.txt", 2048));
    }
    Device_init();
    Scenario_init();
    request->redirect("/?configuration");
  });
  /******************************************************************************
  **********************************UDP******************************************
  ******************************************************************************/
  server.on("/name", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasArg("arg")) {
      jsonWriteStr(configSetup, "name", request->getParam("arg")->value());
      jsonWriteStr(configJson, "name", request->getParam("arg")->value());
      saveConfig();
    }
    request->send(200, "text/text", "OK");
  });
}
