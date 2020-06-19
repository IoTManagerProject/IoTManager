#include "Global.h"

void handleUdp_esp32();
void add_dev_in_list(String fileName, String id, String dev_name, String ip);

#ifdef UDP_enable
void UDP_init() {
  server.on("/udp", HTTP_GET, [](AsyncWebServerRequest * request) {
    String value;
    if (request->hasArg("arg")) {
      value = request->getParam("arg")->value();
    }
    if (value == "0") {
      jsonWriteStr(configSetupJson, "udponoff", value);
      request->send(200, "text/text", "ok");
    }
    if (value == "1") {
      jsonWriteStr(configSetupJson, "udponoff", value);
      request->send(200, "text/text", "ok");
    }
    if (value == "2") {
      mqtt_send_settings_to_udp = true;
      request->send(200, "text/text", "ok");
    }
    if (value == "3") {
      SPIFFS.remove("/dev.csv");
      addFile("dev.csv", "device id;device name;ip address");
      request->redirect("/?dev");
    }
    if (value == "4") {
      request->redirect("/?dev");
    }
  });
  server.on("/name", HTTP_GET, [](AsyncWebServerRequest * request) {
    if (request->hasArg("arg")) {
      jsonWriteStr(configSetupJson, "name", request->getParam("arg")->value());
      jsonWriteStr(configLiveJson, "name", request->getParam("arg")->value());
      saveConfig();
    }
    request->send(200, "text/text", "OK");
  });

  SPIFFS.remove("/dev.csv");
  addFile("dev.csv", "device id;device name;ip address");

#ifdef ESP8266
  Udp.begin(udp_port);
#endif

  handleUdp_esp32();

  randomSeed(micros());
  udp_period = random(50000, 60000);

  ts.add(UDP, udp_period, [&](void*) {
    if (jsonReadStr(configSetupJson, "udponoff") == "1") {
      if (WiFi.status() == WL_CONNECTED) {
        if (!udp_busy) {
          String line_to_send = "iotm;" + chipID + ";" + jsonReadStr(configSetupJson, "name");
#ifdef ESP8266
          Udp.beginPacketMulticast(udp_multicastIP, udp_port, WiFi.localIP());
          Udp.write(line_to_send.c_str());
          Udp.endPacket();
#endif
#ifdef ESP32
          udp.broadcast(line_to_send.c_str());
#endif
          Serial.println("[UDP<=] dev info send");
        }
      }
    }
  }, nullptr, false);
}

void handleUdp() {
#ifdef ESP8266
  if (jsonReadStr(configSetupJson, "udponoff") == "1") {
    if (WiFi.status() == WL_CONNECTED) {
      int packetSize = Udp.parsePacket();
      if (packetSize) {
        char udp_incomingPacket[255];
        Serial.printf("[UDP=>] Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
        received_ip = Udp.remoteIP().toString();
        int len = Udp.read(udp_incomingPacket, 255);
        if (len > 0) {
          udp_incomingPacket[len] = 0;
        }
        received_udp_line = String(udp_incomingPacket);

        if (received_udp_line.indexOf("iotm;") >= 0) {
          udp_data_parse = true;
        }
        if (received_udp_line.indexOf("mqttServer") >= 0) {
          udp_data_parse = true;
        }
      }
    }
  }
#endif
}

void handleUdp_esp32() {
#ifdef ESP32
  if (udp.listenMulticast(udp_multicastIP, udp_port)) {
    udp.onPacket([](AsyncUDPPacket packet) {
      received_udp_line = (char*)packet.data();
      received_ip = packet.remoteIP().toString();
      if (jsonReadStr(configSetupJson, "udponoff") == "1") {

        if (received_udp_line.indexOf("iotm;") >= 0) {
          udp_data_parse = true;
        }
        if (received_udp_line.indexOf("mqttServer") >= 0) {
          udp_data_parse = true;
        }

      }
    });
  }
#endif
}

void do_udp_data_parse() {
  if (udp_data_parse) {
    udp_data_parse = false;
    Serial.print("[UDP=>] " + received_ip);
    Serial.print(" ");
    Serial.println(received_udp_line);
    if (received_udp_line.indexOf("mqttServer") >= 0) {
      jsonWriteStr(configSetupJson, "mqttServer", jsonReadStr(received_udp_line, "mqttServer"));
      jsonWriteInt(configSetupJson, "mqttPort", jsonReadInt(received_udp_line, "mqttPort"));
      jsonWriteStr(configSetupJson, "mqttPrefix", jsonReadStr(received_udp_line, "mqttPrefix"));
      jsonWriteStr(configSetupJson, "mqttUser", jsonReadStr(received_udp_line, "mqttUser"));
      jsonWriteStr(configSetupJson, "mqttPass", jsonReadStr(received_udp_line, "mqttPass"));
      saveConfig();
      Serial.println("[V] new mqtt setting received from udp and saved");
      mqtt_connection = true;
    }
    if (received_udp_line.indexOf("iotm;") >= 0) {
      add_dev_in_list("dev.csv", selectFromMarkerToMarker(received_udp_line, ";", 1), selectFromMarkerToMarker(received_udp_line, ";", 2), received_ip);
    }
  }
}

void add_dev_in_list(String fileName, String id, String dev_name, String ip) {
  File configFile = SPIFFS.open("/" + fileName, "r");
  if (!configFile.find(id.c_str())) {
    addFile(fileName, id + ";" + dev_name + "; <a href=\"http://" + ip + "\" target=\"_blank\"\">" + ip + "</a>");
  }
}

void send_mqtt_to_udp() {
  if (jsonReadStr(configSetupJson, "udponoff") == "1") {
    if (WiFi.status() == WL_CONNECTED) {
      udp_busy = true;
      String mqtt_data = "{}";
      jsonWriteStr(mqtt_data, "mqttServer", jsonReadStr(configSetupJson, "mqttServer"));
      jsonWriteInt(mqtt_data, "mqttPort", jsonReadInt(configSetupJson, "mqttPort"));
      jsonWriteStr(mqtt_data, "mqttPrefix", jsonReadStr(configSetupJson, "mqttPrefix"));
      jsonWriteStr(mqtt_data, "mqttUser", jsonReadStr(configSetupJson, "mqttUser"));
      jsonWriteStr(mqtt_data, "mqttPass", jsonReadStr(configSetupJson, "mqttPass"));
      Serial.println(mqtt_data);
#ifdef ESP8266
      Udp.beginPacketMulticast(udp_multicastIP, udp_port, WiFi.localIP());
      Udp.write(mqtt_data.c_str());
      Udp.endPacket();
#endif
#ifdef ESP32
      udp.broadcast(mqtt_data.c_str());
#endif
      Serial.println("[UDP<=] mqtt info send");
      udp_busy = false;
    }
  }
}

void do_mqtt_send_settings_to_udp() {
  if (mqtt_send_settings_to_udp) {
    mqtt_send_settings_to_udp = false;
    send_mqtt_to_udp();
  }
}
#endif
