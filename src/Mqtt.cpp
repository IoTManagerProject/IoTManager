#include "Global.h"

void callback(char* topic, byte* payload, unsigned int length);

String stateMQTT();
void sendAllData();
void sendAllWigets();
void sendSTATUS(String topik, String state);
void outcoming_date();

//===============================================ИНИЦИАЛИЗАЦИЯ================================================
void MQTT_init() {
  ts.add(WIFI_MQTT_CONNECTION_CHECK, wifi_mqtt_reconnecting, [&](void*) {
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("[VV] WiFi-ok");
      if (client_mqtt.connected()) {
        Serial.println("[VV] MQTT-ok");
        led_blink("off");
      } else {
        MQTT_Connecting();
        if (!just_load) mqtt_lost_error++;
      }
    } else {
      Serial.println("[E] Lost WiFi connection");
      wifi_lost_error++;
      ts.remove(WIFI_MQTT_CONNECTION_CHECK);
      StartAPMode();
    }
  }, nullptr, true);
}

void do_mqtt_connection() {
  if (mqtt_connection) {
    mqtt_connection = false;
    client_mqtt.disconnect();
    MQTT_Connecting();
  }
}

//================================================ОБНОВЛЕНИЕ====================================================
void  handleMQTT() {
  if (WiFi.status() == WL_CONNECTED) {
    if (client_mqtt.connected()) {
      client_mqtt.loop();
    }
  }
}
boolean MQTT_Connecting() {
    bool res = false;
    String mqtt_server = jsonReadStr(configSetupJson, "mqttServer");
    if ((mqtt_server != "")) {
        Serial.println("[E] Lost MQTT connection, start reconnecting");
        led_blink("fast");
        client_mqtt.setServer(mqtt_server.c_str(), jsonReadInt(configSetupJson, "mqttPort"));
        if (WiFi.status() == WL_CONNECTED) {
            if (!client_mqtt.connected()) {
                Serial.println("[V] Connecting to MQTT server commenced");
                if (client_mqtt.connect(chipID.c_str(), jsonReadStr(configSetupJson, "mqttUser").c_str(), jsonReadStr(configSetupJson, "mqttPass").c_str())) {
                    Serial.println("[VV] MQTT connected");
                    led_blink("off");
                    client_mqtt.setCallback(callback);
                    client_mqtt.subscribe(jsonReadStr(configSetupJson, "mqttPrefix").c_str());                                  // Для приема получения HELLOW и подтверждения связи
                    client_mqtt.subscribe((jsonReadStr(configSetupJson, "mqttPrefix") + "/" + chipID + "/+/control").c_str());  // Подписываемся на топики control
                    client_mqtt.subscribe((jsonReadStr(configSetupJson, "mqttPrefix") + "/" + chipID + "/order").c_str());      // Подписываемся на топики order
                    client_mqtt.subscribe((jsonReadStr(configSetupJson, "mqttPrefix") + "/" + chipID + "/update").c_str());
                    client_mqtt.subscribe((jsonReadStr(configSetupJson, "mqttPrefix") + "/" + chipID + "/devc").c_str());
                    client_mqtt.subscribe((jsonReadStr(configSetupJson, "mqttPrefix") + "/" + chipID + "/devs").c_str());
                    Serial.println("[V] Callback set, subscribe done");
                    res = true;
                } else {
                    Serial.println("[E] try again in " + String(wifi_mqtt_reconnecting / 1000) + " sec");
                    led_blink("fast");
                }
            }
        }
    } else {
        Serial.println("[E] No date for MQTT connection");
    }
    return res;
}


//=====================================================ВХОДЯЩИЕ ДАННЫЕ========================================================
void callback(char* topic, byte * payload, unsigned int length) {
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
  if (topic_str.indexOf("control") > 0) {                                     //IoTmanager/800324-1458415/button-sw2/control 1  //IoTmanager/800324-1458415/button99/control 1
    String topic = selectFromMarkerToMarker(topic_str, "/", 3);               //button1                                      //button99
    topic = add_set(topic);                                                   //buttonSet1                                   //buttonSet99
    String number = selectToMarkerLast(topic, "Set");                         //1                                            //99
    topic.replace(number, "");                                                //buttonSet                                    //buttonSet
    String final_line = topic + " " + number + " " + str;                     //buttonSet 1 1                                //buttonSet 99 1
    order_loop += final_line + ",";
  }

  if (topic_str.indexOf("order") > 0) {
    str.replace("_", " ");
    //Serial.println(str);
    order_loop += str + ",";
  }
  if (topic_str.indexOf("update") > 0) {
    if (str == "1") {
      upgrade = true;
    }
  }
  if (topic_str.indexOf("devc") > 0) {
    writeFile("firmware.c.txt", str);
    Device_init();
  }
  if (topic_str.indexOf("devs") > 0) {
    writeFile("firmware.s.txt", str);
    Scenario_init();
  }
}

//данные которые отправляем при подключении или отбновлении страницы
void outcoming_date() {

  sendAllWigets();
  sendAllData();

#ifdef logging_enable
  choose_log_date_and_send();
#endif

  Serial.println("[V] Sending all date to iot manager completed");
}


//======================================CONFIG==================================================
boolean sendMQTT(String end_of_topik, String data) {
  String topik = jsonReadStr(configSetupJson, "mqttPrefix") + "/" + chipID + "/" + end_of_topik;
  boolean send_status = client_mqtt.beginPublish(topik.c_str(), data.length(), false);
  client_mqtt.print(data);
  client_mqtt.endPublish();
  return send_status;
}

boolean sendCHART(String topik, String data) {
  topik = jsonReadStr(configSetupJson, "mqttPrefix") + "/" + chipID + "/" + topik + "/" + "status";
  boolean send_status = client_mqtt.beginPublish(topik.c_str(), data.length(), false);
  client_mqtt.print(data);
  client_mqtt.endPublish();
  return send_status;
}

boolean sendCHART_test(String topik, String data) {
  topik = jsonReadStr(configSetupJson, "mqttPrefix") + "/" + chipID + "/" + topik + "/" + "status";
  boolean send_status = client_mqtt.publish (topik.c_str(), data.c_str(), false);
  return send_status;
}

//======================================STATUS==================================================
void sendSTATUS(String topik, String state) {
  topik = jsonReadStr(configSetupJson, "mqttPrefix") + "/" + chipID + "/" + topik + "/" + "status";
  String json_ = "{}";
  jsonWriteStr(json_, "status", state);
  client_mqtt.publish (topik.c_str(), json_.c_str(), false);
}

//======================================CONTROL==================================================
void sendCONTROL(String id, String topik, String state) {
  String  all_line = jsonReadStr(configSetupJson, "mqttPrefix") + "/" + id + "/" + topik + "/control";
  client_mqtt.publish (all_line.c_str(), state.c_str(), false);
}

//=====================================================ОТПРАВЛЯЕМ ВИДЖЕТЫ========================================================

#ifdef layout_in_ram
void sendAllWigets() {
  if (all_widgets != "") {
    int counter = 0;
    String line;
    int psn_1 = 0;
    int psn_2;
    do  {
      psn_2 = all_widgets.indexOf("\r\n", psn_1); //\r\n
      line = all_widgets.substring(psn_1, psn_2);
      line.replace("\n", "");
      line.replace("\r\n", "");
      //jsonWriteStr(line, "id", String(counter));
      //jsonWriteStr(line, "pageId", String(counter));
      counter++;
      sendMQTT("config", line);
      Serial.println("[V] " + line);
      psn_1 = psn_2 + 1;
    } while (psn_2 + 2 < all_widgets.length());
    getMemoryLoad("[i] after send all widgets");
  }
}
#endif

#ifndef layout_in_ram
void sendAllWigets() {
  File configFile = SPIFFS.open("/layout.txt", "r");
  if (!configFile) {
    return;
  }
  configFile.seek(0, SeekSet); //поставим курсор в начало файла
  while (configFile.position() != configFile.size()) {
    String widget_to_send = configFile.readStringUntil('\n');
    Serial.println("[V] " + widget_to_send);
    sendMQTT("config", widget_to_send);
  }
}
#endif
//=====================================================ОТПРАВЛЯЕМ ДАННЫЕ В ВИДЖЕТЫ ПРИ ОБНОВЛЕНИИ СТРАНИЦЫ========================================================
void sendAllData() {   //берет строку json и ключи превращает в топики а значения колючей в них посылает

  String current_config = configLiveJson;                      //{"name":"MODULES","lang":"","ip":"192.168.43.60","DS":"34.00","rel1":"1","rel2":"1"}
  getMemoryLoad("[i] after send all date");
  current_config.replace("{", "");
  current_config.replace("}", "");                         //"name":"MODULES","lang":"","ip":"192.168.43.60","DS":"34.00","rel1":"1","rel2":"1"
  current_config += ",";                                   //"name":"MODULES","lang":"","ip":"192.168.43.60","DS":"34.00","rel1":"1","rel2":"1",

  while (current_config.length() != 0) {

    String tmp = selectToMarker (current_config, ",");
    String topic =  selectToMarker (tmp, ":");
    topic.replace("\"", "");
    String state =  selectToMarkerLast (tmp, ":");
    state.replace("\"", "");
    if (topic != "name" && topic != "lang" && topic != "ip" && topic.indexOf("_in") < 0) {
      sendSTATUS(topic, state);
      //Serial.println("-->" + topic + " " + state);
    }
    current_config = deleteBeforeDelimiter(current_config, ",");
  }
}



String stateMQTT() {
    int state = client_mqtt.state();
    switch (state) {
        case -4:
            return "the server didn't respond within the keepalive time";
            break;
        case -3:
            return "the network connection was broken";
            break;
        case -2:
            return "the network connection failed";
            break;
        case -1:
            return "the client is disconnected cleanly";
            break;
        case 0:
            return "the client is connected";
            break;
        case 1:
            return "the server doesn't support the requested version of MQTT";
            break;
        case 2:
            return "the server rejected the client identifier";
            break;
        case 3:
            return "the server was unable to accept the connection";
            break;
        case 4:
            return "the username/password were rejected";
            break;
        case 5:
            return "the client was not authorized to connect";
            break;
        default:
            return "unspecified";
            break;
    }
}
