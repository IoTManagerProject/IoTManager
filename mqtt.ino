//===============================================ИНИЦИАЛИЗАЦИЯ================================================
void MQTT_init() {

  server.on("/mqttSave", HTTP_GET, [](AsyncWebServerRequest * request) {

    if (request->hasArg("mqttServer")) {
    jsonWrite(configSetup, "mqttServer", request->getParam("mqttServer")->value());
    }
    if (request->hasArg("mqttPort")) {
    int port = (request->getParam("mqttPort")->value()).toInt();
    jsonWrite(configSetup, "mqttPort", port);
    }
    if (request->hasArg("mqttUser")) {
    jsonWrite(configSetup, "mqttUser", request->getParam("mqttUser")->value());
    }
    if (request->hasArg("mqttPass")) {
    jsonWrite(configSetup, "mqttPass", request->getParam("mqttPass")->value());
    }

    saveConfig();

    client.disconnect();
    MQTT_Connecting(false);

    String tmp = "{}";
    jsonWrite(tmp, "title", "<button class=\"close\" onclick=\"toggle('my-block')\">×</button>" + stateMQTT());
    jsonWrite(tmp, "class", "pop-up");

    request->send(200, "text/text", tmp); // отправляем ответ о выполнении
  });



  //проверка подключения к серверу
  ts.add(WIFI_MQTT_CONNECTION_CHECK, wifi_mqtt_reconnecting, [&](void*) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("[V] WiFi-ok");
      if (client.connected()) {
        Serial.println("[V] MQTT-ok");
      } else {
        MQTT_Connecting(true);
      }
    } else {
      Serial.println("[E] Lost WiFi connection");
      ts.remove(WIFI_MQTT_CONNECTION_CHECK);
      StartAPMode();
    }
  }, nullptr, true);
}

//================================================ОБНОВЛЕНИЕ====================================================

void  handleMQTT() {
  if (WiFi.status() == WL_CONNECTED) {
    if (client.connected()) {
      client.loop();
    }
  }
}
//===============================================ПОДКЛЮЧЕНИЕ========================================================
void MQTT_Connecting(boolean out_date_send) {

  String mqtt_server = jsonRead(configSetup, "mqttServer");

  if ((mqtt_server != "")) {

    static boolean first = true;
    if (!first) Serial.println("[E] Lost MQTT connection, start reconnecting");
    first = false;

    //ssl//espClient.setCACert(local_root_ca1);

    client.setServer(mqtt_server.c_str(), jsonReadtoInt(configSetup, "mqttPort"));
    // подключаемся к MQTT серверу
    if (WiFi.status() == WL_CONNECTED) {
      if (!client.connected()) {
        Serial.println("[V] Connecting to MQTT server commenced");
        //busy = true;
        if (client.connect(chipID.c_str(), jsonRead(configSetup, "mqttUser").c_str(), jsonRead(configSetup, "mqttPass").c_str())) {
          Serial.println("[V] MQTT connected");

          client.setCallback(callback);

          client.subscribe(prefix.c_str());  // Для приема получения HELLOW и подтверждения связи
          client.subscribe((prefix + "/" + chipID + "/+/control").c_str()); // Подписываемся на топики control
          client.subscribe((prefix + "/" + chipID + "/order").c_str()); // Подписываемся на топики order
          //client.subscribe((prefix + "/" + chipID + "/test").c_str());  //Для приема получения work и подтверждения связи (для приложения mqtt IOT MQTT Panel)
          /* String tmp_line = id_of_other_device;

            while (tmp_line.length() != 0) {

             String id = selectToMarker(tmp_line, ",");  //2058631-1589487 1
             id = selectFromMarkerToMarker(id, " ", 0);
             client.subscribe((prefix + "/" + id + "/+/status").c_str(), 0);
             Serial.println("->subscribed to device, id: " + id);

             tmp_line = deleteBeforeDelimiter(tmp_line, ",");
            }*/

          client.subscribe((prefix + "/ids").c_str()); // Подписываемся на топики ids
          sendMQTT("test", "work");
          Serial.println("[V] Callback set, subscribe done");
          //busy = false;
          if (out_date_send) outcoming_date(); //отправляем данные в виджеты
        } else {
          //Serial.println(stateMQTT());
          Serial.println("[E] try again in " + String(wifi_mqtt_reconnecting / 1000) +  " sec");
        }
      }
    }
  } else {
    Serial.println("[E] No date for MQTT connection");
  }
}

//=====================================================ВХОДЯЩИЕ ДАННЫЕ========================================================

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("[MQTT] ");
  Serial.print(topic);
  String topic_str = String(topic);

  String str;
  for (int i = 0; i < length; i++) {
    str += (char)payload[i];
  }
  Serial.println(" => " + str);

  if (str == "HELLO") outcoming_date();
  
  //превращает название топика в команду, а значение в параметр команды

  if (topic_str.indexOf("control") > 0) {                        //IoTmanager/800324-1458415/RelaySet1/control 1   /IoTmanager/9139530-1458400/RelaySet1/control -> 1
    //Serial.println(topic_str);
    String topic = selectFromMarkerToMarker(topic_str, "/", 3);  //RelaySet1
    String number = selectToMarkerLast(topic, "Set");            //1
    topic.replace(number, "");                                   //RelaySet
    String final_line = topic + " " + number + " " + str;        //RelaySet 1 1
    //Serial.println(final_line);
    order_loop += final_line + ",";

  }
  if (topic_str.indexOf("order") > 0) {
    str.replace("_", " ");
    //Serial.println(str);
    order_loop += str + ",";
  }
}

//данные которые отправляем при подключении или отбновлении страницы
void outcoming_date() {

  //busy = true;

  sendAllWigets();
  sendAllData();

  if (flagLoggingAnalog) sendLogData("log.analog.txt", "loganalog");
  if (flagLoggingPh) sendLogData("log.ph.txt", "logph");
  if (flagLoggingDallas) sendLogData("log.dallas.txt", "logdallas");
  if (flagLoggingLevel) sendLogData("log.level.txt", "loglevel");

  Serial.println("[V] Sending all date to iot manager completed");

  //busy = false;

}
//======================================CONFIG==================================================
///IoTmanager/2058631-1589487/config {----viget----}
///sendMQTT("config", data);
boolean sendMQTT(String end_of_topik, String data) {
  String topik = prefix + "/" + chipID + "/" + end_of_topik;
  boolean send_status = client.beginPublish(topik.c_str(), data.length(), false);
  client.print(data);
  client.endPublish();
  return send_status;
}
//======================================STATUS==================================================
///IoTmanager/2058631-1589487/rel1/status {"status":"1"}
///sendSTATUS(topic, state)
void sendSTATUS(String topik, String state) {
  topik = prefix + "/" + chipID + "/" + topik + "/" + "status";
  String json_ = "{}";
  jsonWrite(json_, "status", state);

  //long st_time = millis();
  int send_status =  client.publish (topik.c_str(), json_.c_str(), false);
  //long end_time = millis();
  //Serial.println("send status = " + String(send_status) + ", timeout = " + String(end_time - st_time));
}
/*void sendSTATUS(String topik, String state, String type, String param) {
  topik = prefix + "/" + chipID + "/" + topik + "/" + "status";
  String json_ = "{}";
  jsonWrite(json_, "status", state);
  jsonWrite(json_, type, param);

  //long st_time = millis();
  int send_status =  client.publish (topik.c_str(), json_.c_str(), false);
  //long end_time = millis();
  //Serial.println("send status = " + String(send_status) + ", timeout = " + String(end_time - st_time));
  }*/
//======================================CONTROL==================================================
///IoTmanager/2058631-1589487/rel1/control 1
void sendCONTROL(String id, String topik, String state) {
  String  all_line = prefix + "/" + id + "/" + topik + "/control";

  //long st_time = millis();
  int send_status = client.publish (all_line.c_str(), state.c_str(), false);
  //long end_time = millis();
  //Serial.println("send control = " + String(send_status) + ", timeout = " + String(end_time - st_time));
}

//=====================================================ОТПРАВЛЯЕМ ВИДЖЕТЫ========================================================
void sendAllWigets() {
  if (all_vigets != "") {
    int counter = 0;
    String line;
    int psn_1 = 0;
    int psn_2;

    do  {

      psn_2 = all_vigets.indexOf("\r\n", psn_1);
      line = all_vigets.substring(psn_1, psn_2);
      line.replace("\n", "");
      line.replace("\r\n", "");
      //jsonWrite(line, "id", String(counter));
      //jsonWrite(line, "pageId", String(counter));
      counter++;
      sendMQTT("config",line);
      Serial.println("[V] " + line);
      psn_1 = psn_2 + 1;

    } while (psn_2 + 2 < all_vigets.length());

    getMemoryLoad("[i] after send all vigets");
  }
}
//=====================================================ОТПРАВЛЯЕМ ДАННЫЕ В ВИДЖЕТЫ ПРИ ОБНОВЛЕНИИ СТРАНИЦЫ========================================================

void sendAllData() {   //берет строку json и ключи превращает в топики а значения колючей в них посылает

  String current_config = configJson;                      //{"SSDP":"MODULES","lang":"","ip":"192.168.43.60","DS":"34.00","rel1":"1","rel2":"1"}
  getMemoryLoad("[i] after send all date");
  current_config.replace("{", "");
  current_config.replace("}", "");                         //"SSDP":"MODULES","lang":"","ip":"192.168.43.60","DS":"34.00","rel1":"1","rel2":"1"
  current_config += ",";                                   //"SSDP":"MODULES","lang":"","ip":"192.168.43.60","DS":"34.00","rel1":"1","rel2":"1",

  while (current_config.length() != 0) {

    String tmp = selectToMarker (current_config, ",");
    String topic =  selectToMarker (tmp, ":");
    topic.replace("\"", "");
    String state =  selectToMarkerLast (tmp, ":");
    state.replace("\"", "");
    if (topic != ssdpS && topic != "lang" && topic != "ip" && topic.indexOf("_in") < 0) {
      sendSTATUS(topic, state);
      //Serial.println("-->" + topic + " " + state);
    }
    current_config = deleteBeforeDelimiter(current_config, ",");
  }
}

void sendLogData(String file, String topic) {

  String log_date = readFile(file, 5000) + "\r\n";
  getMemoryLoad("[i] after send log date");

  log_date.replace("\r\n", "\n");
  log_date.replace("\r", "\n");

  while (log_date.length() != 0) {
    String tmp = selectToMarker (log_date, "\n");

    //sendSTATUS(topic, selectFromMarkerToMarker(tmp, " ", 2));
    if (tmp != "") sendSTATUS(topic, tmp);

    log_date = deleteBeforeDelimiter(log_date, "\n");
  }
}

String stateMQTT() {
  
  int state = client.state();

  switch (state) {
    case -4: return "the server didn't respond within the keepalive time";
      break;
    case -3: return "the network connection was broken";
      break;
    case -2: return "the network connection failed";
      break;
    case -1: return "the client is disconnected cleanly";
      break;
    case 0: return "the client is connected";
      break;
    case 1: return "the server doesn't support the requested version of MQTT";
      break;
    case 2: return "the server rejected the client identifier";
      break;
    case 3: return "the server was unable to accept the connection";
      break;
    case 4: return "the username/password were rejected";
      break;
    case 5: return "the client was not authorized to connect";
      break;
  }
}
/*void scenario_devices_topiks_subscribe() {

  //SCENARIO ANALOG > 5 800324-1458415 rel1 0
  if (jsonRead(configSetup, "scenario") == "1") {
    //String all_text = readFile("scenario.all.txt", 1024) + "\r\n";
    String all_text = scenario + "\r\n";
    all_text.replace("\r\n", "\n");
    all_text.replace("\r", "\n");
    while (all_text.length() != 0) {
      String line_ = selectToMarker (all_text, "\n");
      String id = selectFromMarkerToMarker(line_, " ", 4);
      if (id != "not found") {
        client.subscribe((prefix + "/" + id + "/+/status").c_str(), 0);
        Serial.println("subscribed to device, id: " + id);
      }
      all_text = deleteBeforeDelimiter(all_text, "\n");
    }
  }
  }
*/
/*void scenario_devices_test_msg_send() {

  if (jsonRead(configSetup, "scenario") == "1") {

    String all_text = scenario + "\r\n";
    all_text.replace("\r\n", "\n");
    all_text.replace("\r", "\n");
    while (all_text.length() != 0) {
      String line_ = selectToMarker (all_text, "\n");
      String id = selectFromMarkerToMarker(line_, " ", 4);
      if (id != "not found") {
        //Serial.println();
        Serial.println(client.publish ((prefix + "/" + id).c_str(), "CHECK", true));

      }
      all_text = deleteBeforeDelimiter(all_text, "\n");
    }
  }
  }*/

/*
      //-----------------------------------------------------------------------------------------------------------------------------------------------
      //jsonWrite(tmp, "status", "1");

      String current_config = configJson;                  //{"SSDP":"MODULES","lang":"","ip":"192.168.43.60","DS":"34.00","rel1":"1","rel2":"1"}
      current_config.replace("{", "");
      current_config.replace("}", "");                      //"SSDP":"MODULES","lang":"","ip":"192.168.43.60","DS":"34.00","rel1":"1","rel2":"1"
      current_config += ",";                                //"SSDP":"MODULES","lang":"","ip":"192.168.43.60","DS":"34.00","rel1":"1","rel2":"1",

      while (current_config.length() != 0) {

        String tmp = selectToMarker (current_config, ",");  //"rel1":"1"
        String topic =  selectToMarker (tmp, ":");          //"rel1"
        topic.replace("\"", "");                            //rel1
        Serial.println(topic);
        String state =  selectToMarkerLast (tmp, ":");      //"1"
        state.replace("\"", "");                            //1

        //if (viget.lastIndexOf(topic) > 0) {
          jsonWrite(tmp, "status", state);
        //}
        current_config = deleteBeforeDelimiter(current_config, ",");
      }
     //-------------------------------------------------------------------------------------------------------------------------------------------------
*/
